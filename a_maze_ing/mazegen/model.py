"""Maze data model: wall bits, directions and coherent wall updates.

A cell stores its four walls as a four-bit integer. A set bit means the
wall is closed, a clear bit means the passage is open.

    bit 0 (1)  -> North
    bit 1 (2)  -> East
    bit 2 (4)  -> South
    bit 3 (8)  -> West

A value of 15 (F) means the cell is fully closed.
"""

from __future__ import annotations

from dataclasses import dataclass, field

N, E, S, W = 1, 2, 4, 8

# letter: (dx, dy, wall bit of the cell, wall bit of the neighbour)
MOVES = {
    "N": (0, -1, N, S),
    "E": (1, 0, E, W),
    "S": (0, 1, S, N),
    "W": (-1, 0, W, E),
}

WALL_NAME = {"N": "north", "E": "east", "S": "south", "W": "west"}

FULL_WALL = N | E | S | W  # 15 -> all four walls closed


@dataclass
class Cell:
    """A single maze cell."""

    x: int
    y: int
    walls: int = FULL_WALL
    closed: bool = False

    @property
    def is_open(self) -> bool:
        """True when the cell is a traversable corridor."""
        return not self.closed


@dataclass
class Maze:
    """A rectangular grid of cells with coherent walls."""

    width: int
    height: int
    entry: tuple[int, int]
    exit: tuple[int, int]
    grid: list[list[Cell]] = field(default_factory=list)
    blocked: set[tuple[int, int]] = field(default_factory=set)

    def __post_init__(self) -> None:
        """Build the grid of cells when none was provided."""
        if not self.grid:
            self.grid = [
                [Cell(x, y) for x in range(self.width)]
                for y in range(self.height)
            ]

    def cell(self, x: int, y: int) -> Cell:
        """Return the cell at (x, y)."""
        return self.grid[y][x]

    def inside(self, x: int, y: int) -> bool:
        """True when (x, y) is inside the grid."""
        return 0 <= x < self.width and 0 <= y < self.height

    def is_traversable(self, x: int, y: int) -> bool:
        """True when the cell is a corridor (not blocked / not closed)."""
        return self.inside(x, y) and (x, y) not in self.blocked

    def remove_wall(self, x: int, y: int, letter: str) -> None:
        """Open the wall of (x, y) in direction `letter`, coherently."""
        dx, dy, bit, opp = MOVES[letter]
        nx, ny = x + dx, y + dy
        self.grid[y][x].walls &= ~bit
        if self.inside(nx, ny):
            self.grid[ny][nx].walls &= ~opp

    def add_wall(self, x: int, y: int, letter: str) -> None:
        """Close the wall of (x, y) in direction `letter`, coherently."""
        dx, dy, bit, opp = MOVES[letter]
        nx, ny = x + dx, y + dy
        self.grid[y][x].walls |= bit
        if self.inside(nx, ny):
            self.grid[ny][nx].walls |= opp

    def passage(self, x: int, y: int, letter: str) -> bool:
        """True when the passage from (x, y) towards `letter` is open."""
        dx, dy, bit, opp = MOVES[letter]
        nx, ny = x + dx, y + dy
        if not self.inside(nx, ny):
            return False
        return not self.grid[y][x].walls & bit and not self.grid[ny][nx].walls & opp

    def neighbours(self, x: int, y: int) -> list[tuple[int, int, str]]:
        """List the traversable neighbours of (x, y) with the move letter."""
        out: list[tuple[int, int, str]] = []
        for letter, (dx, dy, _bit, _opp) in MOVES.items():
            nx, ny = x + dx, y + dy
            if self.is_traversable(nx, ny) and self.passage(x, y, letter):
                out.append((nx, ny, letter))
        return out

    def open_degree(self, x: int, y: int) -> int:
        """Number of open passages of a traversable cell."""
        return len(self.neighbours(x, y))
