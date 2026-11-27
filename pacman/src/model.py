"""
Core data types for the Pacman game.

These are pure data holders with no I/O, so every rule of the game can be
unit-tested deterministically. The game engine (``game.py``) composes them.
"""
from enum import Enum


class Direction(Enum):
    """The four cardinal directions."""
    NORTH = "N"
    EAST = "E"
    SOUTH = "S"
    WEST = "W"

    @property
    def vector(self):
        return {
            Direction.NORTH: (-1, 0),
            Direction.EAST: (0, 1),
            Direction.SOUTH: (1, 0),
            Direction.WEST: (0, -1),
        }[self]

    @property
    def opposite(self):
        return {
            Direction.NORTH: Direction.SOUTH,
            Direction.EAST: Direction.WEST,
            Direction.SOUTH: Direction.NORTH,
            Direction.WEST: Direction.EAST,
        }[self]


# Map a Direction to the maze wall flag (see maze.py).
DIRECTION_TO_WALL = {
    Direction.NORTH: 1,
    Direction.EAST: 2,
    Direction.SOUTH: 4,
    Direction.WEST: 8,
}
WALL_TO_DIRECTION = {v: k for k, v in DIRECTION_TO_WALL.items()}


class Position:
    """A (row, col) coordinate on the maze grid."""

    __slots__ = ("row", "col")

    def __init__(self, row, col):
        self.row = row
        self.col = col

    def __eq__(self, other):
        return isinstance(other, Position) and self.row == other.row and self.col == other.col

    def __hash__(self):
        return hash((self.row, self.col))

    def __repr__(self):
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

    def __init__(self, kind, position, direction=Direction.NORTH):
        self.kind = kind
        self.position = position
        self.direction = direction
        self.mode = GhostMode.CHASE
        self.alive = True          # False while returning home after being eaten
        self.frightened_timer = 0  # remaining frightened time (ticks)
        self.dead_timer = 0        # remaining time before respawn
        self.target = None         # target Position for pathfinding
        self.score = 0             # points earned by this ghost being eaten

    def __repr__(self):
        return f"Ghost({self.kind.value}, {self.position}, {self.mode.value})"


class Pacgum:
    """A collectible pellet placed on a cell."""

    __slots__ = ("position", "is_super")

    def __init__(self, position, is_super=False):
        self.position = position
        self.is_super = is_super

    def __repr__(self):
        return f"Pacgum({self.position}, super={self.is_super})"


class Pacman:
    """The player-controlled character."""

    def __init__(self, position, direction=Direction.EAST):
        self.position = position
        self.direction = direction
        self.alive = True

    def __repr__(self):
        return f"Pacman({self.position}, {self.direction.value})"


class GameStatus(Enum):
    """The high-level state of a game session."""
    MENU = "menu"
    PLAYING = "playing"
    PAUSED = "paused"
    LEVEL_COMPLETE = "level_complete"
    GAME_OVER = "game_over"
    VICTORY = "victory"
