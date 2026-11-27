"""
Maze generation and the maze model for Pacman.

A maze is a grid of ``Cell`` objects. Each cell stores which of its four
walls (north/east/south/west) are present, encoded as bit flags. The maze is
generated with a recursive-backtracker that produces a *perfect* maze, then a
few internal walls are randomly removed so the result is **non-perfect**: it
contains loops and no dead ends, which gives Pac-Man-style corridors.

The generation is deterministic: passing the same ``seed`` reproduces the same
layout. A **tunnel** connects the left and right borders of the maze so a
character that exits on one side reappears on the other.
"""
import random

from model import Direction, DIRECTION_TO_WALL

# Wall bit flags for a cell.
NORTH = 1
EAST = 2
SOUTH = 4
WEST = 8

# Opposite directions (for wall removal between two cells).
OPPOSITE = {NORTH: SOUTH, EAST: WEST, SOUTH: NORTH, WEST: EAST}

# Direction vectors: (dr, dc) for N, E, S, W.
DIR_VEC = {
    NORTH: (-1, 0),
    EAST: (0, 1),
    SOUTH: (1, 0),
    WEST: (0, -1),
}


def norm_dir(direction):
    """Normalize a ``Direction`` enum (or an int flag) to the int wall flag."""
    if direction in DIR_VEC:
        return direction
    try:
        return DIRECTION_TO_WALL[direction]
    except (KeyError, TypeError):
        return direction


class Cell:
    """A single grid cell with its four walls."""

    __slots__ = ("walls",)

    def __init__(self, walls=NORTH | EAST | SOUTH | WEST):
        self.walls = walls

    def has_wall(self, direction):
        return bool(self.walls & direction)

    def remove_wall(self, direction):
        self.walls &= ~direction

    def add_wall(self, direction):
        self.walls |= direction

    def walls_present(self):
        """Return the list of wall directions that are present."""
        return [d for d in (NORTH, EAST, SOUTH, WEST) if self.has_wall(d)]


class Maze:
    """A generated maze: a grid of cells plus its dimensions."""

    def __init__(self, width, height, seed=0, perfect=False):
        if width < 1 or height < 1:
            raise ValueError("maze dimensions must be positive")
        self.width = width
        self.height = height
        self.seed = seed
        self.perfect = perfect
        self.grid = self._generate(width, height, seed, perfect)

    # ------------------------------------------------------------------ generation
    def _generate(self, width, height, seed, perfect):
        rng = random.Random(seed)
        # Every cell starts fully walled.
        grid = [[Cell() for _ in range(width)] for _ in range(height)]

        visited = [[False] * width for _ in range(height)]
        stack = [(0, 0)]
        visited[0][0] = True

        while stack:
            r, c = stack[-1]
            # Collect unvisited neighbours.
            neighbours = []
            for direction, (dr, dc) in DIR_VEC.items():
                nr, nc = r + dr, c + dc
                if 0 <= nr < height and 0 <= nc < width and not visited[nr][nc]:
                    neighbours.append((direction, nr, nc))
            if neighbours:
                direction, nr, nc = rng.choice(neighbours)
                # Remove the shared wall between the two cells.
                grid[r][c].remove_wall(direction)
                grid[nr][nc].remove_wall(OPPOSITE[direction])
                visited[nr][nc] = True
                stack.append((nr, nc))
            else:
                stack.pop()

        if not perfect:
            self._add_loops(grid, rng)
        return grid

    def _add_loops(self, grid, rng):
        """Remove a few internal walls to create loops (non-perfect maze)."""
        # Try to remove a small fraction of the internal walls.
        removals = max(1, (self.width * self.height) // 8)
        for _ in range(removals):
            r = rng.randrange(self.height)
            c = rng.randrange(self.width)
            cell = grid[r][c]
            # Only remove an internal wall (one that has a neighbour in the
            # maze, i.e. not the outer border) to keep the maze enclosed.
            candidates = []
            for direction, (dr, dc) in DIR_VEC.items():
                nr, nc = r + dr, c + dc
                if 0 <= nr < self.height and 0 <= nc < self.width:
                    if cell.has_wall(direction):
                        candidates.append(direction)
            if candidates:
                direction = rng.choice(candidates)
                cell.remove_wall(direction)
                nr, nc = r + DIR_VEC[direction][0], c + DIR_VEC[direction][1]
                grid[nr][nc].remove_wall(OPPOSITE[direction])

    # ------------------------------------------------------------------ access
    def get(self, row, col):
        return self.grid[row][col]

    def in_bounds(self, row, col):
        return 0 <= row < self.height and 0 <= col < self.width

    def is_passable(self, row, col, direction):
        """Whether a character can move from (row, col) in ``direction``."""
        direction = norm_dir(direction)
        if not self.in_bounds(row, col):
            return False
        # Tunnel: crossing the left/right border wraps around, regardless of
        # the border wall (it is an opening on the whole side).
        if direction == WEST and col == 0:
            return True
        if direction == EAST and col == self.width - 1:
            return True
        cell = self.grid[row][col]
        if cell.has_wall(direction):
            return False
        dr, dc = DIR_VEC[direction]
        nr, nc = row + dr, col + dc
        return self.in_bounds(nr, nc)

    def move(self, row, col, direction):
        """Return the (row, col) reached by moving in ``direction``.

        Handles the tunnel wrap-around. Call only after ``is_passable``.
        """
        direction = norm_dir(direction)
        dr, dc = DIR_VEC[direction]
        nr, nc = row + dr, col + dc
        if not self.in_bounds(nr, nc):
            if direction == WEST and col == 0:
                return row, self.width - 1
            if direction == EAST and col == self.width - 1:
                return row, 0
        return nr, nc

    def neighbors(self, row, col):
        """Return the list of (direction, row, col) passable neighbours."""
        result = []
        for direction in (NORTH, EAST, SOUTH, WEST):
            if self.is_passable(row, col, direction):
                nr, nc = self.move(row, col, direction)
                result.append((direction, nr, nc))
        return result

    # ------------------------------------------------------------------ properties
    @property
    def pacgums_positions(self):
        """Return all cell positions (row, col) where a pac-gum can be placed.

        The corners and the border cells are excluded to keep the layout
        clean; in practice the caller chooses the actual pac-gum positions.
        """
        positions = []
        for r in range(self.height):
            for c in range(self.width):
                # Skip the very border ring.
                if r in (0, self.height - 1) or c in (0, self.width - 1):
                    continue
                positions.append((r, c))
        return positions

    def to_grid(self):
        """Return the raw grid (list of list of Cell) for serialization."""
        return self.grid
