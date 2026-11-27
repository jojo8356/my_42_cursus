"""End-to-end integration tests for the full game flow and bonuses."""
import random

import pytest

from maze import Maze
from model import (
    Direction, Ghost, GhostKind, GhostMode, Pacgum, Pacman, Position,
    GameStatus,
)
from game import Game
from highscore import HighscoreStore


def _make_game(width=10, height=10, seed=42, lives=3, level_time=100000):
    maze = Maze(width, height, seed=seed, perfect=False)
    pacman = Pacman(Position(height // 2, width // 2), Direction.EAST)
    ghosts = [
        Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH),
        Ghost(GhostKind.PINKY, Position(0, width - 1), Direction.SOUTH),
        Ghost(GhostKind.INKY, Position(height - 1, width - 1), Direction.WEST),
        Ghost(GhostKind.CLYDE, Position(height - 1, 0), Direction.NORTH),
    ]
    pacgums = [Pacgum(Position(r, c)) for r, c in maze.pacgums_positions]
    return Game(maze, pacman, ghosts, pacgums, lives=lives,
                level_time=level_time, rng=random.Random(seed))


def test_full_game_clears_level_and_victory():
    """Clearing all pellets completes the level; clearing the last reaches victory."""
    g = _make_game()
    # Neutralize ghosts so Pac-Man can roam freely.
    for ghost in g.ghosts:
        ghost.mode = GhostMode.SCATTER
    # Eat every pellet.
    for pg in list(g.pacgums):
        g.pacman.position = Position(pg.position.row, pg.position.col)
        g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    g.step()
    assert g.status == GameStatus.LEVEL_COMPLETE
    assert g.is_victory(1) is True  # single level -> victory


def test_multi_level_progression_to_victory():
    """Advancing through all levels ends in victory."""
    max_level = 3
    g = _make_game()
    for ghost in g.ghosts:
        ghost.mode = GhostMode.SCATTER
    for expected_level in range(1, max_level + 1):
        assert g.level == expected_level
        for pg in list(g.pacgums):
            g.pacman.position = Position(pg.position.row, pg.position.col)
            g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
        g.step()
        assert g.status == GameStatus.LEVEL_COMPLETE
        if expected_level < max_level:
            g.next_level()
    assert g.status == GameStatus.LEVEL_COMPLETE
    assert g.is_victory(max_level) is True


def test_tunnel_movement_in_game():
    """Pac-Man can cross the left/right tunnel during a real step."""
    g = _make_game(width=8, height=8, seed=1)
    # Place Pac-Man on the left border and move west to wrap east.
    g.pacman.position = Position(4, 0)
    g.pacman.direction = Direction.WEST
    g.set_input(Direction.WEST)
    g.step()
    assert g.pacman.position.col == g.maze.width - 1
    assert g.pacman.position.row == 4


def test_score_and_lives_persist_across_level():
    """Score carries over when advancing levels."""
    g = _make_game()
    g.score = 1234
    for ghost in g.ghosts:
        ghost.mode = GhostMode.SCATTER
    for pg in list(g.pacgums):
        g.pacman.position = Position(pg.position.row, pg.position.col)
        g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    g.step()
    score_before = g.score
    g.next_level()
    assert g.score == score_before  # score persists
    assert g.status == GameStatus.PLAYING


def test_highscore_records_final_score(tmp_path):
    """A final game score can be recorded into the high-score store."""
    g = _make_game()
    for ghost in g.ghosts:
        ghost.mode = GhostMode.SCATTER
    for pg in list(g.pacgums):
        g.pacman.position = Position(pg.position.row, pg.position.col)
        g.eat_pacgum_at(g.pacman.position.row, g.pacman.position.col)
    final_score = g.score

    store = HighscoreStore(str(tmp_path / "hs.json"))
    store.load()
    assert store.is_new_record(final_score) is True
    store.add("PacPlayer", final_score)
    store.save()

    store2 = HighscoreStore(str(tmp_path / "hs.json"))
    store2.load()
    assert store2.top()[0] == {"name": "PacPlayer", "score": final_score}


def test_frightened_ghost_can_be_eaten_and_respawns():
    """A frightened ghost eaten turns dead, then returns home and revives."""
    g = _make_game()
    ghost = g.ghosts[0]
    ghost.mode = GhostMode.FRIGHTENED
    g.pacman.position = Position(ghost.position.row, ghost.position.col)
    g.check_collisions()
    assert ghost.mode == GhostMode.DEAD
    assert g.score == g.points_per_ghost
    # Move the dead ghost home and revive it.
    home = Position(g.maze.height - 1, g.maze.width // 2)
    ghost.position = home
    g.move_ghosts()
    assert ghost.mode == GhostMode.CHASE
    assert ghost.alive is True


def test_no_unhandled_errors_over_many_ticks():
    """Running many ticks with random inputs never raises."""
    g = _make_game(width=12, height=12, seed=7)
    directions = [Direction.NORTH, Direction.EAST, Direction.SOUTH, Direction.WEST]
    for i in range(500):
        if g.status != GameStatus.PLAYING:
            break
        g.set_input(directions[i % 4])
        g.step()
    # The engine must end in a valid terminal or ongoing state.
    assert g.status in (GameStatus.PLAYING, GameStatus.GAME_OVER,
                        GameStatus.LEVEL_COMPLETE, GameStatus.VICTORY)


def test_timer_does_not_go_negative_after_game_over():
    """Once game over, the timer stops decreasing."""
    g = _make_game()
    g.lives = 1
    ghost = g.ghosts[0]
    ghost.mode = GhostMode.CHASE
    g.pacman.position = Position(ghost.position.row, ghost.position.col)
    g.check_collisions()
    assert g.status == GameStatus.GAME_OVER
    timer_before = g.timer
    for _ in range(10):
        g.step()
    assert g.timer == timer_before
