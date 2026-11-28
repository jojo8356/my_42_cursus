"""
Maze generation and the maze model for Pacman.

A maze is a grid of ``Cell`` objects. Each cell stores which of its four
walls (north/east/south/west) are present, encoded as bit flags. The maze is
generated with a recursive-backtracker that produces a perfect maze, then a
few internal walls are randomly removed so the result contains loops.

The generation is deterministic: passing the same ``seed`` reproduces the same
layout. A tunnel connects the left and right borders of the maze so a
character that exits on one side reappears on the other.
"""
from __future__ import annotations

import random
from typing import TypeAlias

from model import Direction, DIRECTION_TO_WALL

# Wall bit flags for a cell.
NORTH = 1
EAST = 2
SOUTH = 4
WEST = 8

# Opposite directions (for wall removal between two cells).
OPPOSITE: dict[int, int] = {
    NORTH: SOUTH,
    EAST: WEST,
    SOUTH: NORTH,
    WEST: EAST,
}

# Direction vectors: (dr, dc) for N, E, S, W.
DIR_VEC: dict[int, tuple[int, int]] = {
    NORTH: (-1, 0),
    EAST: (0, 1),
    SOUTH: (1, 0),
    WEST: (0, -1),
}

DirectionInput: TypeAlias = Direction | int
Coordinate: TypeAlias = tuple[int, int]


def norm_dir(direction: DirectionInput) -> int:
    """Convert a ``Direction`` or wall flag to a wall flag.

    Invalid values are rejected instead of silently being passed on to code
    that indexes the direction vectors.
    """
    if isinstance(direction, Direction):
        return DIRECTION_TO_WALL[direction]
    if type(direction) is int and direction in DIR_VEC:
        return direction
    raise ValueError(f"Invalid direction: {direction!r}")


class Cell:
    """A single grid cell with its four walls."""

    __slots__ = ("walls",)

    def __init__(self, walls: int = NORTH | EAST | SOUTH | WEST) -> None:
        self.walls = walls

    def has_wall(self, direction: int) -> bool:
        return bool(self.walls & direction)

    def remove_wall(self, direction: int) -> None:
        self.walls &= ~direction

    def add_wall(self, direction: int) -> None:
        self.walls |= direction

    def walls_present(self) -> list[int]:
        """Return the list of wall directions that are present."""
        return [
            direction
            for direction in (NORTH, EAST, SOUTH, WEST)
            if self.has_wall(direction)
        ]


class Maze:
    """A generated maze: a grid of cells plus its dimensions."""

    def __init__(
        self,
        width: int,
        height: int,
        seed: int = 0,
        perfect: bool = False,
    ) -> None:
        if (
            type(width) is not int
            or type(height) is not int
            or width < 1
            or height < 1
        ):
            raise ValueError("maze dimensions must be positive integers")
        self.width = width
        self.height = height
        self.seed = seed
        self.perfect = perfect
        self.grid = self._generate(width, height, seed, perfect)

    def _generate(
        self,
        width: int,
        height: int,
        seed: int,
        perfect: bool,
    ) -> list[list[Cell]]:
        rng = random.Random(seed)
        grid = [[Cell() for _ in range(width)] for _ in range(height)]

        visited = [[False] * width for _ in range(height)]
        stack: list[Coordinate] = [(0, 0)]
        visited[0][0] = True

        while stack:
            row, col = stack[-1]
            neighbours: list[tuple[int, int, int]] = []
            for direction, (dr, dc) in DIR_VEC.items():
                next_row, next_col = row + dr, col + dc
                if (
                    0 <= next_row < height
                    and 0 <= next_col < width
                    and not visited[next_row][next_col]
                ):
                    neighbours.append((direction, next_row, next_col))
            if neighbours:
                direction, next_row, next_col = rng.choice(neighbours)
                grid[row][col].remove_wall(direction)
                grid[next_row][next_col].remove_wall(OPPOSITE[direction])
                visited[next_row][next_col] = True
                stack.append((next_row, next_col))
            else:
                stack.pop()

        if not perfect:
            self._add_loops(grid, rng)
        return grid

    def _add_loops(self, grid: list[list[Cell]], rng: random.Random) -> None:
        """Remove a few internal walls to create loops."""
        removals = max(1, (self.width * self.height) // 8)
        for _ in range(removals):
            row = rng.randrange(self.height)
            col = rng.randrange(self.width)
            cell = grid[row][col]
            candidates = []
            for direction, (dr, dc) in DIR_VEC.items():
                next_row, next_col = row + dr, col + dc
                if (
                    0 <= next_row < self.height
                    and 0 <= next_col < self.width
                    and cell.has_wall(direction)
                ):
                    candidates.append(direction)
            if candidates:
                direction = rng.choice(candidates)
                cell.remove_wall(direction)
                dr, dc = DIR_VEC[direction]
                grid[row + dr][col + dc].remove_wall(OPPOSITE[direction])

    def get(self, row: int, col: int) -> Cell:
        """Return the cell at a row and column."""
        return self.grid[row][col]

    def in_bounds(self, row: int, col: int) -> bool:
        """Whether a row and column identify a cell in the maze."""
        return 0 <= row < self.height and 0 <= col < self.width

    def is_passable(
        self,
        row: int,
        col: int,
        direction: DirectionInput,
    ) -> bool:
        """Whether a character can move from a cell in ``direction``."""
        direction_flag = norm_dir(direction)
        if not self.in_bounds(row, col):
            return False
        if direction_flag == WEST and col == 0:
            return True
        if direction_flag == EAST and col == self.width - 1:
            return True
        cell = self.grid[row][col]
        if cell.has_wall(direction_flag):
            return False
        dr, dc = DIR_VEC[direction_flag]
        next_row, next_col = row + dr, col + dc
        return self.in_bounds(next_row, next_col)

    def move(
        self,
        row: int,
        col: int,
        direction: DirectionInput,
    ) -> Coordinate:
        """Return the cell reached by moving in ``direction``.

        Handles the tunnel wrap-around. Call only after ``is_passable``.
        """
        direction_flag = norm_dir(direction)
        dr, dc = DIR_VEC[direction_flag]
        next_row, next_col = row + dr, col + dc
        if not self.in_bounds(next_row, next_col):
            if direction_flag == WEST and col == 0:
                return row, self.width - 1
            if direction_flag == EAST and col == self.width - 1:
                return row, 0
        return next_row, next_col

    def neighbors(self, row: int, col: int) -> list[tuple[int, int, int]]:
        """Return passable neighbours as (wall flag, row, column) tuples."""
        result = []
        for direction in (NORTH, EAST, SOUTH, WEST):
            if self.is_passable(row, col, direction):
                next_row, next_col = self.move(row, col, direction)
                result.append((direction, next_row, next_col))
        return result

    @property
    def pacgums_positions(self) -> list[Coordinate]:
        """Return all non-border cells where pac-gums may be placed."""
        positions = []
        for row in range(self.height):
            for col in range(self.width):
                if row in (0, self.height - 1) or col in (0, self.width - 1):
                    continue
                positions.append((row, col))
        return positions

    def to_grid(self) -> list[list[Cell]]:
        """Return the raw grid (list of list of Cell) for serialization."""
        return self.grid
