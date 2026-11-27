"""Tests for the core data model."""
import pytest

from model import (
    Direction, Position, Pacman, Ghost, GhostKind, GhostMode, Pacgum,
    GameStatus,
)


def test_direction_vectors():
    assert Direction.NORTH.vector == (-1, 0)
    assert Direction.EAST.vector == (0, 1)
    assert Direction.SOUTH.vector == (1, 0)
    assert Direction.WEST.vector == (0, -1)


def test_direction_opposite():
    assert Direction.NORTH.opposite == Direction.SOUTH
    assert Direction.SOUTH.opposite == Direction.NORTH
    assert Direction.EAST.opposite == Direction.WEST
    assert Direction.WEST.opposite == Direction.EAST


def test_position_equality_hash():
    p1 = Position(3, 4)
    p2 = Position(3, 4)
    p3 = Position(4, 3)
    assert p1 == p2
    assert p1 != p3
    assert hash(p1) == hash(p2)
    assert p1 in {p2}


def test_pacman_defaults():
    p = Pacman(Position(1, 1))
    assert p.position == Position(1, 1)
    assert p.direction == Direction.EAST
    assert p.alive is True


def test_ghost_defaults():
    g = Ghost(GhostKind.BLINKY, Position(0, 0))
    assert g.kind == GhostKind.BLINKY
    assert g.position == Position(0, 0)
    assert g.mode == GhostMode.CHASE
    assert g.alive is True


def test_pacgum_super_flag():
    pg_normal = Pacgum(Position(1, 1), is_super=False)
    pg_super = Pacgum(Position(1, 1), is_super=True)
    assert pg_normal.is_super is False
    assert pg_super.is_super is True


def test_game_status_values():
    assert GameStatus.PLAYING.value == "playing"
    assert GameStatus.GAME_OVER.value == "game_over"
    assert GameStatus.VICTORY.value == "victory"
    assert GameStatus.LEVEL_COMPLETE.value == "level_complete"
    assert GameStatus.PAUSED.value == "paused"
    assert GameStatus.MENU.value == "menu"


def test_ghost_mode_values():
    assert GhostMode.CHASE.value == "chase"
    assert GhostMode.SCATTER.value == "scatter"
    assert GhostMode.FRIGHTENED.value == "frightened"
    assert GhostMode.DEAD.value == "dead"


def test_ghost_kind_values():
    assert GhostKind.BLINKY.value == "blinky"
    assert GhostKind.PINKY.value == "pinky"
    assert GhostKind.INKY.value == "inky"
    assert GhostKind.CLYDE.value == "clyde"
