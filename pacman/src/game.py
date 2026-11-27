"""
The Pacman game engine.

A pure, deterministic, I/O-free engine: it owns the maze, Pac-Man, the four
ghosts, the pac-gums, the score, lives, the level timer and the frightened
timer. Every rule is expressed as methods so it can be unit-tested without a
screen or a keyboard.

Movement is cell-based: on each ``step()`` (one tick) Pac-Man and the ghosts
move at most one cell, constrained by the maze walls (and the tunnel). The
engine resolves pellet collection, ghost collisions, frightened mode, lives,
the timer and level/game/victory transitions.
"""
import random

from maze import Maze
from model import (
    Direction, Ghost, GhostKind, GhostMode, Pacgum, Pacman, Position,
    GameStatus,
)
from ghosts import choose_direction

# Default tuning (overridable via GameConfig).
DEFAULT_POINTS_PACGUM = 10
DEFAULT_POINTS_SUPER = 50
DEFAULT_POINTS_GHOST = 200
DEFAULT_LIVES = 3
DEFAULT_LEVEL_TIME = 90 * 60  # 90 seconds at 60 ticks/second
DEFAULT_FRIGHTENED_TICKS = 300  # ~5 seconds at 60 ticks/second
DEFAULT_EXTRA_LIFE_SCORE = 10000
DEFAULT_FRUIT_PACGUM_THRESHOLD = 70


class Game:
    """A single playable game session."""

    def __init__(self, maze, pacman, ghosts, pacgums, *,
                 points_per_pacgum=DEFAULT_POINTS_PACGUM,
                 points_per_super_pacgum=DEFAULT_POINTS_SUPER,
                 points_per_ghost=DEFAULT_POINTS_GHOST,
                 lives=DEFAULT_LIVES,
                 level_time=DEFAULT_LEVEL_TIME,
                 frightened_ticks=DEFAULT_FRIGHTENED_TICKS,
                 extra_life_score=DEFAULT_EXTRA_LIFE_SCORE,
                 fruit_threshold=DEFAULT_FRUIT_PACGUM_THRESHOLD,
                 rng=None):
        self.maze = maze
        self.pacman = pacman
        self.ghosts = ghosts
        self.pacgums = list(pacgums)
        self.points_per_pacgum = points_per_pacgum
        self.points_per_super_pacgum = points_per_super_pacgum
        self.points_per_ghost = points_per_ghost
        self.lives = lives
        self.level_time = level_time
        self.frightened_ticks = frightened_ticks
        self.extra_life_score = extra_life_score
        self.fruit_threshold = fruit_threshold
        self.rng = rng or random.Random(0)

        self.score = 0
        self.level = 1
        self.status = GameStatus.PLAYING
        self.timer = level_time
        self.pacman_input = None       # desired Direction from the player
        self.frightened_ticks = frightened_ticks
        self.frightened_remaining = frightened_ticks  # ticks of frightened active
        self.fruit_eaten = 0           # pac-gums eaten so far (for fruit)
        self.fruit_present = False
        self.fruit_position = None
        self.next_extra_life = extra_life_score

        # Set the initial ghost modes.
        for ghost in self.ghosts:
            ghost.mode = GhostMode.CHASE

    # ------------------------------------------------------------------ helpers
    def _passable(self, row, col, direction):
        return self.maze.is_passable(row, col, direction)

    def _remaining_pacgums(self):
        return len(self.pacgums)

    # ------------------------------------------------------------------ input
    def set_input(self, direction):
        """Set the direction the player wants to move (or None to hold)."""
        self.pacman_input = direction

    # ------------------------------------------------------------------ pacman
    def move_pacman(self):
        if self.status != GameStatus.PLAYING or not self.pacman.alive:
            return
        direction = self.pacman_input
        if direction is None:
            return
        # Only turn into a passable direction; if the desired direction is
        # blocked, try to keep moving in the current direction.
        candidate = direction
        if not self._passable(self.pacman.position.row, self.pacman.position.col, candidate):
            # Try the current direction as a fallback.
            if self._passable(self.pacman.position.row, self.pacman.position.col,
                              self.pacman.direction):
                candidate = self.pacman.direction
            else:
                return
        self.pacman.direction = candidate
        nr, nc = self.maze.move(self.pacman.position.row,
                                self.pacman.position.col, candidate)
        self.pacman.position = Position(nr, nc)

    # ------------------------------------------------------------------ pacgums
    def eat_pacgum_at(self, row, col):
        """Remove and score the pac-gum at (row, col), if any."""
        for i, pg in enumerate(self.pacgums):
            if pg.position.row == row and pg.position.col == col:
                self.pacgums.pop(i)
                if pg.is_super:
                    self.score += self.points_per_super_pacgum
                    self._activate_frightened()
                else:
                    self.score += self.points_per_pacgum
                self.fruit_eaten += 1
                self._maybe_spawn_fruit()
                self._check_extra_life()
                return True
        return False

    def _activate_frightened(self):
        self.frightened_remaining = self.frightened_ticks
        for ghost in self.ghosts:
            if ghost.mode != GhostMode.DEAD:
                ghost.mode = GhostMode.FRIGHTENED
                ghost.frightened_timer = self.frightened_ticks

    def _check_extra_life(self):
        while self.score >= self.next_extra_life:
            self.lives += 1
            self.next_extra_life += self.extra_life_score

    def _maybe_spawn_fruit(self):
        if self.fruit_threshold and self.fruit_eaten == self.fruit_threshold:
            self.fruit_present = True
            self.fruit_position = self._random_empty_cell()

    def _random_empty_cell(self):
        # Place the fruit somewhere in the middle of the maze.
        h, w = self.maze.height, self.maze.width
        for _ in range(20):
            r = self.rng.randrange(max(1, h - 1))
            c = self.rng.randrange(max(1, w - 1))
            if self._remaining_pacgums() or True:
                return Position(r, c)
        return Position(h // 2, w // 2)

    # ------------------------------------------------------------------ ghosts
    def move_ghosts(self):
        for ghost in self.ghosts:
            if ghost.mode == GhostMode.DEAD:
                # Move along the shortest path home.
                if self._dead_ghost_arrived(ghost):
                    ghost.mode = GhostMode.CHASE
                    ghost.alive = True
                    ghost.dead_timer = 0
                    continue
                self._move_dead_ghost(ghost)
                continue

            # Skip frightened tick handling here; handled in step().
            direction = choose_direction(
                ghost, self.maze, self.pacman.position.row,
                self.pacman.position.col, self.pacman.direction, self.rng,
            )
            if self._passable(ghost.position.row, ghost.position.col, direction):
                nr, nc = self.maze.move(ghost.position.row,
                                        ghost.position.col, direction)
                ghost.position = Position(nr, nc)
                ghost.direction = direction

    def _dead_ghost_arrived(self, ghost):
        h, w = self.maze.height, self.maze.width
        return (ghost.position.row == h - 1 and ghost.position.col == w // 2)

    def _move_dead_ghost(self, ghost):
        from ghosts import shortest_path, direction_between
        path = shortest_path(
            self.maze, ghost.position.row, ghost.position.col,
            self.maze.height - 1, self.maze.width // 2,
        )
        if len(path) >= 2:
            next_cell = path[1]
            direction = direction_between(
                self.maze, ghost.position.row, ghost.position.col, next_cell)
            if self._passable(ghost.position.row, ghost.position.col, direction):
                nr, nc = self.maze.move(ghost.position.row,
                                        ghost.position.col, direction)
                ghost.position = Position(nr, nc)
                ghost.direction = direction

    # ------------------------------------------------------------------ collisions
    def check_collisions(self):
        pr, pc = self.pacman.position.row, self.pacman.position.col
        # Pellet / fruit collection.
        self.eat_pacgum_at(pr, pc)
        if self.fruit_present and self.fruit_position:
            if (self.fruit_position.row == pr
                    and self.fruit_position.col == pc):
                self.score += 100  # fruit bonus
                self.fruit_present = False
                self.fruit_position = None

        for ghost in self.ghosts:
            if ghost.mode == GhostMode.DEAD:
                continue
            gr, gc = ghost.position.row, ghost.position.col
            if gr == pr and gc == pc:
                if ghost.mode == GhostMode.FRIGHTENED:
                    # Eat the frightened ghost.
                    self.score += self.points_per_ghost
                    ghost.mode = GhostMode.DEAD
                    ghost.alive = False
                    ghost.dead_timer = 30
                else:
                    # Lose a life.
                    self._lose_life()
                    return

    def _lose_life(self):
        self.lives -= 1
        if self.lives <= 0:
            self.status = GameStatus.GAME_OVER
            return
        # Respawn: Pac-Man to the centre, ghosts to their corners.
        self._respawn()

    def _respawn(self):
        h, w = self.maze.height, self.maze.width
        self.pacman.position = Position(h // 2, w // 2)
        self.frightened_remaining = 0
        from ghosts import CORNERS
        for ghost in self.ghosts:
            if ghost.mode == GhostMode.DEAD:
                continue
            corn = CORNERS[ghost.kind]
            ghost.position = Position(
                max(0, min(h - 1, corn[0])),
                max(0, min(w - 1, corn[1])),
            )
            ghost.mode = GhostMode.CHASE

    # ------------------------------------------------------------------ step
    def step(self):
        """Advance the game by one tick."""
        if self.status != GameStatus.PLAYING:
            return
        self.move_pacman()
        self.check_collisions()
        if self.status != GameStatus.PLAYING:
            return
        self.move_ghosts()
        self.check_collisions()

        # Frightened timer.
        if self.frightened_remaining > 0:
            self.frightened_remaining -= 1
            if self.frightened_remaining == 0:
                for ghost in self.ghosts:
                    if ghost.mode == GhostMode.FRIGHTENED:
                        ghost.mode = GhostMode.CHASE

        # Timer.
        self.timer -= 1
        if self.timer <= 0:
            self._lose_life()
            if self.status == GameStatus.PLAYING:
                self.timer = self.level_time

        # Level complete?
        if self._remaining_pacgums() == 0 and not self.fruit_present:
            self.status = GameStatus.LEVEL_COMPLETE

    # ------------------------------------------------------------------ transitions
    def next_level(self):
        """Advance to the next level. Returns the new level number."""
        self.level += 1
        self.status = GameStatus.PLAYING
        self.timer = self.level_time
        return self.level

    def is_victory(self, max_level):
        """Whether the game is won (all levels completed)."""
        return self.status == GameStatus.LEVEL_COMPLETE and self.level >= max_level

    def to_state(self):
        """Serialize the engine state for the UI / tests."""
        return {
            "status": self.status.value,
            "score": self.score,
            "lives": self.lives,
            "level": self.level,
            "timer": self.timer,
            "pacman": (self.pacman.position.row, self.pacman.position.col),
            "pacman_dir": self.pacman.direction.value,
            "ghosts": [
                (g.position.row, g.position.col, g.kind.value, g.mode.value)
                for g in self.ghosts
            ],
            "pacgums_left": self._remaining_pacgums(),
        }
