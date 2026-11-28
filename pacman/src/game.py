"""
The Pacman game engine.

A deterministic, I/O-free engine that owns the maze, Pac-Man, the four ghosts,
the pac-gums, score, lives and timers. Each ``step()`` advances the game by one
tick and resolves movement, collections, collisions and level transitions.
"""
from __future__ import annotations

import random
from collections.abc import Sequence

from ghosts import choose_direction, direction_between, shortest_path
from maze import Maze
from model import (
    Direction,
    GameStatus,
    Ghost,
    GhostMode,
    Pacgum,
    Pacman,
    Position,
)

# Default tuning (overridable via GameConfig).
DEFAULT_POINTS_PACGUM = 10
DEFAULT_POINTS_SUPER = 50
DEFAULT_POINTS_GHOST = 200
DEFAULT_LIVES = 3
DEFAULT_LEVEL_TIME = 90 * 60
DEFAULT_FRIGHTENED_TICKS = 300
DEFAULT_EXTRA_LIFE_SCORE = 10000
DEFAULT_FRUIT_PACGUM_THRESHOLD = 70


class Game:
    """A single playable game session."""

    def __init__(
        self,
        maze: Maze,
        pacman: Pacman,
        ghosts: Sequence[Ghost],
        pacgums: Sequence[Pacgum],
        *,
        points_per_pacgum: int = DEFAULT_POINTS_PACGUM,
        points_per_super_pacgum: int = DEFAULT_POINTS_SUPER,
        points_per_ghost: int = DEFAULT_POINTS_GHOST,
        lives: int = DEFAULT_LIVES,
        level_time: int = DEFAULT_LEVEL_TIME,
        frightened_ticks: int = DEFAULT_FRIGHTENED_TICKS,
        extra_life_score: int = DEFAULT_EXTRA_LIFE_SCORE,
        fruit_threshold: int = DEFAULT_FRUIT_PACGUM_THRESHOLD,
        rng: random.Random | None = None,
    ) -> None:
        self.maze = maze
        self.pacman = pacman
        self.ghosts = list(ghosts)
        self.pacgums = list(pacgums)
        self.points_per_pacgum = points_per_pacgum
        self.points_per_super_pacgum = points_per_super_pacgum
        self.points_per_ghost = points_per_ghost
        self.lives = lives
        self.level_time = level_time
        self.frightened_ticks = frightened_ticks
        self.extra_life_score = extra_life_score
        self.fruit_threshold = fruit_threshold
        self.rng = rng if rng is not None else random.Random(0)

        self.score = 0
        self.level = 1
        self.status = GameStatus.PLAYING
        self.timer = level_time
        self.pacman_input: Direction | None = None
        self.frightened_remaining = 0
        self.fruit_eaten = 0
        self.fruit_present = False
        self.fruit_position: Position | None = None
        self.next_extra_life = extra_life_score

        for ghost in self.ghosts:
            ghost.mode = GhostMode.CHASE

    def _passable(self, row: int, col: int, direction: Direction) -> bool:
        return self.maze.is_passable(row, col, direction)

    def _remaining_pacgums(self) -> int:
        return len(self.pacgums)

    def set_input(self, direction: Direction | None) -> None:
        """Set the direction the player wants to move (or None to hold)."""
        self.pacman_input = direction

    def move_pacman(self) -> None:
        if self.status != GameStatus.PLAYING or not self.pacman.alive:
            return
        direction = self.pacman_input
        if direction is None:
            return
        candidate = direction
        if not self._passable(
            self.pacman.position.row,
            self.pacman.position.col,
            candidate,
        ):
            if self._passable(
                self.pacman.position.row,
                self.pacman.position.col,
                self.pacman.direction,
            ):
                candidate = self.pacman.direction
            else:
                return
        self.pacman.direction = candidate
        next_row, next_col = self.maze.move(
            self.pacman.position.row,
            self.pacman.position.col,
            candidate,
        )
        self.pacman.position = Position(next_row, next_col)

    def eat_pacgum_at(self, row: int, col: int) -> bool:
        """Remove and score the pac-gum at a cell, if present."""
        for index, pacgum in enumerate(self.pacgums):
            if pacgum.position.row != row or pacgum.position.col != col:
                continue
            self.pacgums.pop(index)
            if pacgum.is_super:
                self.score += self.points_per_super_pacgum
                self._activate_frightened()
            else:
                self.score += self.points_per_pacgum
            self.fruit_eaten += 1
            self._maybe_spawn_fruit()
            self._check_extra_life()
            return True
        return False

    def _activate_frightened(self) -> None:
        self.frightened_remaining = self.frightened_ticks
        for ghost in self.ghosts:
            if ghost.mode != GhostMode.DEAD:
                ghost.mode = GhostMode.FRIGHTENED
                ghost.frightened_timer = self.frightened_ticks

    def _check_extra_life(self) -> None:
        while self.score >= self.next_extra_life:
            self.lives += 1
            self.next_extra_life += self.extra_life_score

    def _maybe_spawn_fruit(self) -> None:
        if self.fruit_threshold and self.fruit_eaten == self.fruit_threshold:
            self.fruit_present = True
            self.fruit_position = self._random_empty_cell()

    def _random_empty_cell(self) -> Position:
        """Choose a cell not already occupied by a player or collectible."""
        occupied = {(self.pacman.position.row, self.pacman.position.col)}
        occupied.update(
            (ghost.position.row, ghost.position.col)
            for ghost in self.ghosts
        )
        occupied.update(
            (pacgum.position.row, pacgum.position.col)
            for pacgum in self.pacgums
        )
        empty_cells = [
            (row, col)
            for row in range(self.maze.height)
            for col in range(self.maze.width)
            if (row, col) not in occupied
        ]
        if not empty_cells:
            return Position(self.maze.height // 2, self.maze.width // 2)
        row, col = self.rng.choice(empty_cells)
        return Position(row, col)

    def move_ghosts(self) -> None:
        for ghost in self.ghosts:
            if ghost.mode == GhostMode.DEAD:
                if self._dead_ghost_arrived(ghost):
                    ghost.mode = GhostMode.CHASE
                    ghost.alive = True
                    ghost.dead_timer = 0
                    ghost.frightened_timer = 0
                    continue
                self._move_dead_ghost(ghost)
                continue

            direction = choose_direction(
                ghost,
                self.maze,
                self.pacman.position.row,
                self.pacman.position.col,
                self.pacman.direction,
                self.rng,
            )
            if self._passable(
                ghost.position.row,
                ghost.position.col,
                direction,
            ):
                next_row, next_col = self.maze.move(
                    ghost.position.row,
                    ghost.position.col,
                    direction,
                )
                ghost.position = Position(next_row, next_col)
                ghost.direction = direction

    def _dead_ghost_arrived(self, ghost: Ghost) -> bool:
        return (
            ghost.position.row == self.maze.height - 1
            and ghost.position.col == self.maze.width // 2
        )

    def _move_dead_ghost(self, ghost: Ghost) -> None:
        path = shortest_path(
            self.maze,
            ghost.position.row,
            ghost.position.col,
            self.maze.height - 1,
            self.maze.width // 2,
        )
        if len(path) < 2:
            return
        direction = direction_between(
            self.maze,
            ghost.position.row,
            ghost.position.col,
            path[1],
        )
        if self._passable(ghost.position.row, ghost.position.col, direction):
            next_row, next_col = self.maze.move(
                ghost.position.row,
                ghost.position.col,
                direction,
            )
            ghost.position = Position(next_row, next_col)
            ghost.direction = direction

    def check_collisions(self) -> None:
        player_row = self.pacman.position.row
        player_col = self.pacman.position.col
        self.eat_pacgum_at(player_row, player_col)
        if self.fruit_present and self.fruit_position is not None:
            if (
                self.fruit_position.row == player_row
                and self.fruit_position.col == player_col
            ):
                self.score += 100
                self.fruit_present = False
                self.fruit_position = None

        for ghost in self.ghosts:
            if ghost.mode == GhostMode.DEAD:
                continue
            if (
                ghost.position.row != player_row
                or ghost.position.col != player_col
            ):
                continue
            if ghost.mode == GhostMode.FRIGHTENED:
                self.score += self.points_per_ghost
                ghost.mode = GhostMode.DEAD
                ghost.alive = False
                ghost.dead_timer = 30
            else:
                self._lose_life()
                return

    def _lose_life(self) -> None:
        self.lives -= 1
        if self.lives <= 0:
            self.status = GameStatus.GAME_OVER
            return
        self._respawn()

    def _respawn(self) -> None:
        self.pacman.position = Position(
            self.maze.height // 2,
            self.maze.width // 2,
        )
        self.pacman.alive = True
        self.frightened_remaining = 0
        from ghosts import CORNERS

        for ghost in self.ghosts:
            if ghost.mode == GhostMode.DEAD:
                continue
            corner_row, corner_col = CORNERS[ghost.kind]
            if corner_row < 0:
                corner_row = self.maze.height - 1
            if corner_col < 0:
                corner_col = self.maze.width - 1
            ghost.position = Position(
                max(0, min(self.maze.height - 1, corner_row)),
                max(0, min(self.maze.width - 1, corner_col)),
            )
            ghost.mode = GhostMode.CHASE
            ghost.frightened_timer = 0

    def step(self) -> None:
        """Advance the game by one tick."""
        if self.status != GameStatus.PLAYING:
            return
        self.move_pacman()
        self.check_collisions()
        if self.status != GameStatus.PLAYING:
            return
        self.move_ghosts()
        self.check_collisions()

        if self.frightened_remaining > 0:
            self.frightened_remaining -= 1
            if self.frightened_remaining == 0:
                for ghost in self.ghosts:
                    if ghost.mode == GhostMode.FRIGHTENED:
                        ghost.mode = GhostMode.CHASE
                        ghost.frightened_timer = 0

        self.timer -= 1
        if self.timer <= 0:
            self._lose_life()
            if self.status == GameStatus.PLAYING:
                self.timer = self.level_time

        if self._remaining_pacgums() == 0 and not self.fruit_present:
            self.status = GameStatus.LEVEL_COMPLETE

    def next_level(self) -> int:
        """Advance the level counter while preserving session progress."""
        self.level += 1
        self.status = GameStatus.PLAYING
        self.timer = self.level_time
        self.pacman_input = None
        self.frightened_remaining = 0
        self.fruit_eaten = 0
        self.fruit_present = False
        self.fruit_position = None
        return self.level

    def start_next_level(self, next_level: Game) -> int:
        """Replace level-specific state without resetting session progress.

        The score, lives and extra-life threshold belong to the overall game
        session and are intentionally preserved. The maze, actors, collectibles
        and level timers come from ``next_level`` and are freshly initialized.
        """
        self.maze = next_level.maze
        self.pacman = next_level.pacman
        self.pacman.alive = True
        self.ghosts = list(next_level.ghosts)
        self.pacgums = list(next_level.pacgums)
        self.points_per_pacgum = next_level.points_per_pacgum
        self.points_per_super_pacgum = next_level.points_per_super_pacgum
        self.points_per_ghost = next_level.points_per_ghost
        self.level_time = next_level.level_time
        self.frightened_ticks = next_level.frightened_ticks
        self.extra_life_score = next_level.extra_life_score
        self.fruit_threshold = next_level.fruit_threshold
        self.rng = next_level.rng

        self.level += 1
        self.status = GameStatus.PLAYING
        self.timer = self.level_time
        self.pacman_input = None
        self.frightened_remaining = 0
        self.fruit_eaten = 0
        self.fruit_present = False
        self.fruit_position = None
        for ghost in self.ghosts:
            ghost.mode = GhostMode.CHASE
            ghost.alive = True
            ghost.frightened_timer = 0
            ghost.dead_timer = 0
            ghost.target = None
        return self.level

    def is_victory(self, max_level: int) -> bool:
        """Whether all configured levels have been completed."""
        return (
            self.status == GameStatus.LEVEL_COMPLETE
            and self.level >= max_level
        )

    def to_state(self) -> dict[str, object]:
        """Serialize the engine state for the UI and tests."""
        return {
            "status": self.status.value,
            "score": self.score,
            "lives": self.lives,
            "level": self.level,
            "timer": self.timer,
            "pacman": (self.pacman.position.row, self.pacman.position.col),
            "pacman_dir": self.pacman.direction.value,
            "ghosts": [
                (ghost.position.row, ghost.position.col,
                 ghost.kind.value, ghost.mode.value)
                for ghost in self.ghosts
            ],
            "pacgums_left": self._remaining_pacgums(),
        }
