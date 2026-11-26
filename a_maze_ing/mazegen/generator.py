"""Maze generation: DFS spanning tree, the "42" pattern and braiding."""

from __future__ import annotations

import random

from .errors import ValidationError
from .model import FULL_WALL, MOVES, Maze
from .solver import shortest_path
from .validator import (
    open_3x3,
    validate,
)

# 7x5 bitmap drawing the digits "4" and "2" with fully closed cells.
PATTERN = [
    "X.X.XXX",
    "X.X...X",
    "XXX.XXX",
    "..X.X..",
    "..X.XXX",
]
PATTERN_W = len(PATTERN[0])
PATTERN_H = len(PATTERN)


def _pattern_cells(ox: int, oy: int) -> set[tuple[int, int]]:
    """Set of blocked cells for the pattern placed at its top-left (ox, oy)."""
    blocked: set[tuple[int, int]] = set()
    for dy, row in enumerate(PATTERN):
        for dx, char in enumerate(row):
            if char == "X":
                blocked.add((ox + dx, oy + dy))
    return blocked


def _non_pattern_connected(width: int, height: int,
                           blocked: set[tuple[int, int]]) -> bool:
    """True when every non-blocked cell is 4-connected to the entry."""
    start = next(
        (x, y) for y in range(height) for x in range(width)
        if (x, y) not in blocked
    )
    seen = {start}
    stack = [start]
    while stack:
        x, y = stack.pop()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if (0 <= n[0] < width and 0 <= n[1] < height
                    and n not in blocked and n not in seen):
                seen.add(n)
                stack.append(n)
    return len(seen) == width * height - len(blocked)


class MazeGenerator:
    """Generate a valid maze, reusable independently of the CLI.

    Parameters
    ----------
    width, height:
        Dimensions of the grid in cells.
    entry, exit:
        (x, y) coordinates of the entry and the exit.
    perfect:
        True -> a perfect maze (a single path, no loop). False -> a
        Pac-Man-like board with loops, open corners and centre.
    seed:
        Optional integer for reproducible random generation.
    algorithm:
        "dfs" (default) or "prim".
    include_pattern:
        Try to embed the visible "42" of fully closed cells.
    """

    def __init__(
        self,
        width: int,
        height: int,
        entry: tuple[int, int],
        exit: tuple[int, int],
        perfect: bool = True,
        seed: int | None = None,
        algorithm: str = "dfs",
        include_pattern: bool = True,
    ) -> None:
        """Store the parameters and generate the first maze."""
        self.width = width
        self.height = height
        self.entry = (int(entry[0]), int(entry[1]))
        self.exit = (int(exit[0]), int(exit[1]))
        self.perfect = perfect
        self.seed = seed
        self.algorithm = algorithm
        self.include_pattern = include_pattern
        self.maze: Maze | None = None
        self.path: str = ""
        self.forty_two: list[tuple[int, int]] = []
        self.pattern_omitted = False
        self.generate()

    # ------------------------------------------------------------------
    def generate(self) -> Maze:
        """Generate the maze, solve it and store it in ``self.maze``."""
        rng = random.Random(self.seed)
        blocked = self._choose_pattern(rng)
        self.forty_two = sorted(blocked)
        self.pattern_omitted = not blocked
        maze = Maze(self.width, self.height, self.entry, self.exit, blocked=blocked)
        for x, y in blocked:
            maze.cell(x, y).closed = True
            maze.cell(x, y).walls = FULL_WALL
        if self.algorithm == "prim":
            self._generate_prim(maze, rng)
        else:
            self._generate_dfs(maze, rng)
        if not self.perfect:
            self._braid(maze, rng)
        self.maze = maze
        self.path = shortest_path(maze)
        problems = validate(maze, self.perfect)
        if problems:
            raise ValidationError("; ".join(problems))
        return maze

    def solve(self) -> str:
        """Return the shortest path from the entry to the exit."""
        if self.maze is None:
            self.generate()
        assert self.maze is not None
        return shortest_path(self.maze)

    @property
    def walls(self) -> list[list[int]]:
        """The grid of wall values (one integer per cell)."""
        if self.maze is None:
            self.generate()
        assert self.maze is not None
        return [
            [self.maze.cell(x, y).walls for x in range(self.width)]
            for y in range(self.height)
        ]

    # ------------------------------------------------------------------
    def _choose_pattern(self, rng: random.Random) -> set[tuple[int, int]]:
        """Pick a safe placement for the "42", or return an empty set."""
        if not self.include_pattern:
            return set()
        if self.width < PATTERN_W + 2 or self.height < PATTERN_H + 2:
            return set()
        corners = {(0, 0), (self.width - 1, 0),
                   (0, self.height - 1), (self.width - 1, self.height - 1)}
        centres = self._centres()
        candidates: list[tuple[int, int]] = []
        cx = (self.width - PATTERN_W) // 2
        cy = (self.height - PATTERN_H) // 2
        for dy in range(-2, 3):
            for dx in range(-2, 3):
                ox, oy = cx + dx, cy + dy
                if 1 <= ox and ox + PATTERN_W - 1 < self.width - 1:
                    if 1 <= oy and oy + PATTERN_H - 1 < self.height - 1:
                        candidates.append((ox, oy))
        rng.shuffle(candidates)
        for ox, oy in candidates:
            blocked = _pattern_cells(ox, oy)
            if self.entry in blocked or self.exit in blocked:
                continue
            if blocked & corners:
                continue
            if blocked >= centres:
                continue
            if not _non_pattern_connected(self.width, self.height, blocked):
                continue
            return blocked
        return set()

    def _centres(self) -> set[tuple[int, int]]:
        """The up-to-four central cells of the grid."""
        xs = sorted({(self.width - 1) // 2, self.width // 2})
        ys = sorted({(self.height - 1) // 2, self.height // 2})
        return {(x, y) for y in ys for x in xs}

    # ------------------------------------------------------------------
    def _carve(self, maze: Maze, rng: random.Random) -> None:
        """Open the walls of a spanning tree over the traversable cells."""
        start = self.entry
        visited: set[tuple[int, int]] = {start}
        stack: list[tuple[int, int]] = [start]
        while stack:
            x, y = stack[-1]
            options: list[tuple[int, int, str]] = []
            for letter, (dx, dy, _bit, _opp) in MOVES.items():
                nx, ny = x + dx, y + dy
                if maze.is_traversable(nx, ny) and (nx, ny) not in visited:
                    options.append((nx, ny, letter))
            if options:
                nx, ny, letter = rng.choice(options)
                maze.remove_wall(x, y, letter)
                visited.add((nx, ny))
                stack.append((nx, ny))
            else:
                stack.pop()

    def _generate_dfs(self, maze: Maze, rng: random.Random) -> None:
        """Recursive-backtracker (iterative DFS)."""
        self._carve(maze, rng)

    def _generate_prim(self, maze: Maze, rng: random.Random) -> None:
        """Randomized Prim's algorithm."""
        start = self.entry
        visited: set[tuple[int, int]] = {start}
        frontier: list[tuple[int, int, int, int, str]] = []
        for letter, (dx, dy, _bit, _opp) in MOVES.items():
            nx, ny = start[0] + dx, start[1] + dy
            if maze.is_traversable(nx, ny):
                frontier.append((nx, ny, start[0], start[1], letter))
        while frontier:
            nx, ny, ox, oy, letter = rng.choice(frontier)
            frontier = [f for f in frontier if (f[0], f[1]) != (nx, ny)]
            if (nx, ny) in visited:
                continue
            maze.remove_wall(ox, oy, letter)
            visited.add((nx, ny))
            for l2, (dx, dy, _bit, _opp) in MOVES.items():
                tx, ty = nx + dx, ny + dy
                if maze.is_traversable(tx, ty) and (tx, ty) not in visited:
                    frontier.append((tx, ty, nx, ny, l2))

    # ------------------------------------------------------------------
    def _would_create_3x3(self, maze: Maze, x: int, y: int,
                          letter: str) -> bool:
        """True when opening `letter` at (x, y) creates a forbidden 3x3."""
        maze.remove_wall(x, y, letter)
        bad = bool(open_3x3(maze))
        dx, dy, bit, opp = MOVES[letter]
        maze.add_wall(x, y, letter)
        return bad

    def _braid(self, maze: Maze, rng: random.Random) -> None:
        """Open extra walls to create loops and reduce dead-ends."""
        for _ in range(self.width * self.height * 2):
            dead = [
                (x, y) for y in range(self.height) for x in range(self.width)
                if maze.is_traversable(x, y) and maze.open_degree(x, y) == 1
            ]
            if len(dead) <= 2:
                break
            rng.shuffle(dead)
            progress = False
            for x, y in dead:
                if len([
                    (a, b) for a in range(self.width) for b in range(self.height)
                    if maze.is_traversable(a, b) and maze.open_degree(a, b) == 1
                ]) <= 2:
                    break
                options: list[tuple[int, int, str]] = []
                for letter, (dx, dy, _bit, _opp) in MOVES.items():
                    nx, ny = x + dx, y + dy
                    if not maze.is_traversable(nx, ny):
                        continue
                    if maze.passage(x, y, letter):
                        continue
                    options.append((nx, ny, letter))
                # prefer neighbours that are dead-ends themselves
                options.sort(
                    key=lambda c: 0 if maze.open_degree(*c[:2]) == 1 else 1
                )
                rng.shuffle(options)
                for nx, ny, letter in options:
                    if not self._would_create_3x3(maze, x, y, letter):
                        maze.remove_wall(x, y, letter)
                        progress = True
                        break
                if progress:
                    break
            if not progress:
                break
