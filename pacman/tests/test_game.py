"""Tests for the game engine."""
import random

import pytest

from maze import Maze
from model import (
    Direction, Ghost, GhostKind, GhostMode, Pacgum, Pacman, Position,
    GameStatus,
)
from game import Game


@pytest.fixture
def small_game():
    """A small, deterministic game for engine tests."""
    maze = Maze(6, 6, seed=42, perfect=False)
    pacman = Pacman(Position(3, 3), Direction.EAST)
    ghosts = [
        Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH),
        Ghost(GhostKind.PINKY, Position(0, 5), Direction.SOUTH),
        Ghost(GhostKind.INKY, Position(5, 5), Direction.WEST),
        Ghost(GhostKind.CLYDE, Position(5, 0), Direction.NORTH),
    ]
    pacgums = [
        Pacgum(Position(1, 1), is_super=False),
        Pacgum(Position(1, 2), is_super=False),
        Pacgum(Position(2, 3), is_super=True),
    ]
    game = Game(maze, pacman, ghosts, pacgums, lives=3, level_time=600,
                rng=random.Random(0))
    return game


def test_initial_state(small_game):
    g = small_game
    assert g.status == GameStatus.PLAYING
    assert g.score == 0
    assert g.lives == 3
    assert g.level == 1
    assert g._remaining_pacgums() == 3


def test_pacman_moves(small_game):
    g = small_game
    start = g.pacman.position
    # Find a passable direction from the centre and move.
    for direction in (Direction.NORTH, Direction.EAST,
                      Direction.SOUTH, Direction.WEST):
        if g.maze.is_passable(start.row, start.col, direction):
            g.set_input(direction)
            g.step()
            assert g.pacman.position != start
            return
    pytest.skip("no passable direction from centre")


def test_pacman_blocked_by_wall(small_game):
    g = small_game
    # Find a wall direction from the current cell.
    start = g.pacman.position
    blocked = None
    for direction in (Direction.NORTH, Direction.EAST,
                      Direction.SOUTH, Direction.WEST):
        if not g.maze.is_passable(start.row, start.col, direction):
            blocked = direction
            break
    if blocked is None:
        pytest.skip("no blocked direction")
    # Point Pac-Man into the wall both as input and as its current facing,
    # so neither the desired nor the fallback direction is passable.
    g.pacman.direction = blocked
    g.set_input(blocked)
    g.move_pacman()
    # Pac-Man did not pass through the wall.
    assert g.pacman.position == start


def test_eat_pacgum_scores(small_game):
    g = small_game
    # Place Pac-Man on a pac-gum cell.
    pg = g.pacgums[0]
    g.pacman.position = Position(pg.position.row, pg.position.col)
    assert g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    assert g.score == g.points_per_pacgum
    assert g._remaining_pacgums() == 2


def test_super_pacgum_activates_frightened(small_game):
    g = small_game
    super_pg = [p for p in g.pacgums if p.is_super][0]
    g.pacman.position = Position(super_pg.position.row, super_pg.position.col)
    g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    assert g.score == g.points_per_super_pacgum
    assert g.frightened_remaining > 0
    for ghost in g.ghosts:
        if ghost.mode != GhostMode.DEAD:
            assert ghost.mode == GhostMode.FRIGHTENED


def test_eat_frightened_ghost(small_game):
    g = small_game
    ghost = g.ghosts[0]
    ghost.mode = GhostMode.FRIGHTENED
    g.pacman.position = Position(ghost.position.row, ghost.position.col)
    g.check_collisions()
    assert ghost.mode == GhostMode.DEAD
    assert g.score == g.points_per_ghost


def test_hit_chasing_ghost_loses_life(small_game):
    g = small_game
    ghost = g.ghosts[0]
    ghost.mode = GhostMode.CHASE
    g.pacman.position = Position(ghost.position.row, ghost.position.col)
    lives_before = g.lives
    g.check_collisions()
    assert g.lives == lives_before - 1


def test_game_over_when_no_lives(small_game):
    g = small_game
    g.lives = 1
    ghost = g.ghosts[0]
    ghost.mode = GhostMode.CHASE
    g.pacman.position = Position(ghost.position.row, ghost.position.col)
    g.check_collisions()
    assert g.status == GameStatus.GAME_OVER


def test_timer_expires_loses_life(small_game):
    g = small_game
    g.timer = 1
    lives_before = g.lives
    g.step()
    assert g.lives == lives_before - 1


def test_level_complete_when_all_pellets_eaten(small_game):
    g = small_game
    # Eat all pac-gums by teleporting Pac-Man onto each.
    for pg in list(g.pacgums):
        g.pacman.position = Position(pg.position.row, pg.position.col)
        g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    g.step()
    assert g.status == GameStatus.LEVEL_COMPLETE


def test_next_level(small_game):
    g = small_game
    for pg in list(g.pacgums):
        g.pacman.position = Position(pg.position.row, pg.position.col)
        g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    g.step()
    assert g.status == GameStatus.LEVEL_COMPLETE
    new_level = g.next_level()
    assert new_level == 2
    assert g.status == GameStatus.PLAYING


def test_extra_life_at_threshold(small_game):
    g = small_game
    g.score = g.extra_life_score
    g._check_extra_life()
    assert g.lives == 4  # started with 3, +1 at 10000
    # Second threshold also grants a life.
    g.score = 2 * g.extra_life_score
    g._check_extra_life()
    assert g.lives == 5


def test_fruit_spawns_and_eaten(small_game):
    g = small_game
    g.fruit_eaten = g.fruit_threshold
    g._maybe_spawn_fruit()
    assert g.fruit_present is True
    assert g.fruit_position is not None
    g.pacman.position = Position(g.fruit_position.row, g.fruit_position.col)
    score_before = g.score
    g.check_collisions()
    assert g.score == score_before + 100
    assert g.fruit_present is False


def test_ghost_frightened_timer_expires(small_game):
    g = small_game
    super_pg = [p for p in g.pacgums if p.is_super][0]
    g.pacman.position = Position(super_pg.position.row, super_pg.position.col)
    g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    assert g.frightened_remaining > 0
    # Run enough steps to exhaust the frightened timer.
    for _ in range(g.frightened_ticks + 1):
        if g.status != GameStatus.PLAYING:
            break
        g.step()
    # Ghosts should have left frightened mode.
    for ghost in g.ghosts:
        if ghost.mode != GhostMode.DEAD:
            assert ghost.mode != GhostMode.FRIGHTENED


def test_to_state_serialization(small_game):
    g = small_game
    state = g.to_state()
    assert state["status"] == "playing"
    assert state["score"] == 0
    assert state["lives"] == 3
    assert state["level"] == 1
    assert isinstance(state["ghosts"], list)
    assert state["pacgums_left"] == 3


def test_respawn_after_life_lost(small_game):
    g = small_game
    start_pacman = g.pacman.position
    ghost = g.ghosts[0]
    ghost.mode = GhostMode.CHASE
    g.pacman.position = Position(ghost.position.row, ghost.position.col)
    g.check_collisions()
    assert g.lives == 2
    # Pac-Man respawns at the centre.
    assert g.pacman.position.row == g.maze.height // 2
    assert g.pacman.position.col == g.maze.width // 2


def test_victory_condition(small_game):
    g = small_game
    g.level = 10
    g.status = GameStatus.LEVEL_COMPLETE
    assert g.is_victory(10) is True
    assert g.is_victory(11) is False
