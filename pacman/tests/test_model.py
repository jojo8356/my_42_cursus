"""Tests for the core data model."""
from model import (
    Direction,
    GameStatus,
    Ghost,
    GhostKind,
    GhostMode,
    Pacgum,
    Pacman,
    Position,
)


def test_direction_vectors() -> None:
    assert Direction.NORTH.vector == (-1, 0)
    assert Direction.EAST.vector == (0, 1)
    assert Direction.SOUTH.vector == (1, 0)
    assert Direction.WEST.vector == (0, -1)


def test_direction_opposites() -> None:
    assert Direction.NORTH.opposite == Direction.SOUTH
    assert Direction.SOUTH.opposite == Direction.NORTH
    assert Direction.EAST.opposite == Direction.WEST
    assert Direction.WEST.opposite == Direction.EAST


def test_position_equality_and_hash() -> None:
    first = Position(3, 4)
    second = Position(3, 4)
    different = Position(4, 3)
    assert first == second
    assert first != different
    assert hash(first) == hash(second)
    assert first in {second}


def test_pacman_defaults() -> None:
    pacman = Pacman(Position(1, 1))
    assert pacman.position == Position(1, 1)
    assert pacman.direction == Direction.EAST
    assert pacman.alive is True


def test_ghost_defaults() -> None:
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0))
    assert ghost.kind == GhostKind.BLINKY
    assert ghost.position == Position(0, 0)
    assert ghost.mode == GhostMode.CHASE
    assert ghost.alive is True
    assert ghost._rng is None


def test_pacgum_super_flag() -> None:
    normal_pacgum = Pacgum(Position(1, 1), is_super=False)
    super_pacgum = Pacgum(Position(1, 1), is_super=True)
    assert normal_pacgum.is_super is False
    assert super_pacgum.is_super is True


def test_game_status_values() -> None:
    assert GameStatus.PLAYING.value == "playing"
    assert GameStatus.GAME_OVER.value == "game_over"
    assert GameStatus.VICTORY.value == "victory"
    assert GameStatus.LEVEL_COMPLETE.value == "level_complete"
    assert GameStatus.PAUSED.value == "paused"
    assert GameStatus.MENU.value == "menu"


def test_ghost_mode_values() -> None:
    assert GhostMode.CHASE.value == "chase"
    assert GhostMode.SCATTER.value == "scatter"
    assert GhostMode.FRIGHTENED.value == "frightened"
    assert GhostMode.DEAD.value == "dead"


def test_ghost_kind_values() -> None:
    assert GhostKind.BLINKY.value == "blinky"
    assert GhostKind.PINKY.value == "pinky"
    assert GhostKind.INKY.value == "inky"
    assert GhostKind.CLYDE.value == "clyde"


def test_model_representations() -> None:
    position = Position(1, 2)
    ghost = Ghost(GhostKind.BLINKY, position)
    pacman = Pacman(position)
    pacgum = Pacgum(position)

    assert repr(position) == "Position(1, 2)"
    assert "Ghost(blinky" in repr(ghost)
    assert "Pacman(Position(1, 2)" in repr(pacman)
    assert "Pacgum(Position(1, 2)" in repr(pacgum)
