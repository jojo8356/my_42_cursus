"""Core data types for the Pacman game.

These are pure data holders with no I/O, so every rule of the game can be
unit-tested deterministically. The game engine (``game.py``) composes them.
"""
from __future__ import annotations

import random
from enum import Enum


class Direction(Enum):
    """The four cardinal directions."""

    NORTH = "N"
    EAST = "E"
    SOUTH = "S"
    WEST = "W"

    @property
    def vector(self) -> tuple[int, int]:
        """Return the row and column delta for this direction."""
        return {
            Direction.NORTH: (-1, 0),
            Direction.EAST: (0, 1),
            Direction.SOUTH: (1, 0),
            Direction.WEST: (0, -1),
        }[self]

    @property
    def opposite(self) -> Direction:
        """Return the direction directly opposite this one."""
        return {
            Direction.NORTH: Direction.SOUTH,
            Direction.EAST: Direction.WEST,
            Direction.SOUTH: Direction.NORTH,
            Direction.WEST: Direction.EAST,
        }[self]


# Map a Direction to the maze wall flag (see maze.py).
DIRECTION_TO_WALL: dict[Direction, int] = {
    Direction.NORTH: 1,
    Direction.EAST: 2,
    Direction.SOUTH: 4,
    Direction.WEST: 8,
}
WALL_TO_DIRECTION: dict[int, Direction] = {
    value: key for key, value in DIRECTION_TO_WALL.items()
}


class Position:
    """A (row, col) coordinate on the maze grid."""

    __slots__ = ("row", "col")

    def __init__(self, row: int, col: int) -> None:
        self.row = row
        self.col = col

    def __eq__(self, other: object) -> bool:
        return (
            isinstance(other, Position)
            and self.row == other.row
            and self.col == other.col
        )

    def __hash__(self) -> int:
        return hash((self.row, self.col))

    def __repr__(self) -> str:
        return f"Position({self.row}, {self.col})"


class GhostKind(Enum):
    """The four ghosts, each with its own behaviour."""

    BLINKY = "blinky"
    PINKY = "pinky"
    INKY = "inky"
    CLYDE = "clyde"


class GhostMode(Enum):
    """The behavioural state of a ghost."""

    CHASE = "chase"
    SCATTER = "scatter"
    FRIGHTENED = "frightened"
    DEAD = "dead"


class Ghost:
    """A ghost moving on the grid."""

    def __init__(
        self,
        kind: GhostKind,
        position: Position,
        direction: Direction = Direction.NORTH,
    ) -> None:
        self.kind = kind
        self.position = position
        self.direction = direction
        self.mode = GhostMode.CHASE
        self.alive = True
        self.frightened_timer = 0
        self.dead_timer = 0
        self.target: Position | None = None
        self.score = 0
        self._rng: random.Random | None = None

    def __repr__(self) -> str:
        return f"Ghost({self.kind.value}, {self.position}, {self.mode.value})"


class Pacgum:
    """A collectible pellet placed on a cell."""

    __slots__ = ("position", "is_super")

    def __init__(self, position: Position, is_super: bool = False) -> None:
        self.position = position
        self.is_super = is_super

    def __repr__(self) -> str:
        return f"Pacgum({self.position}, super={self.is_super})"


class Pacman:
    """The player-controlled character."""

    def __init__(
        self,
        position: Position,
        direction: Direction = Direction.EAST,
    ) -> None:
        self.position = position
        self.direction = direction
        self.alive = True

    def __repr__(self) -> str:
        return f"Pacman({self.position}, {self.direction.value})"


class GameStatus(Enum):
    """The high-level state of a game session."""

    MENU = "menu"
    PLAYING = "playing"
    PAUSED = "paused"
    LEVEL_COMPLETE = "level_complete"
    GAME_OVER = "game_over"
    VICTORY = "victory"
