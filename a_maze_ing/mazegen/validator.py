"""Structural validation of a generated maze."""

from __future__ import annotations

from .model import MOVES, Maze
from .solver import bfs


def wall_mismatch(maze: Maze) -> list[tuple[int, int, str]]:
    """Shared walls encoded differently between two neighbouring cells."""
    bad: list[tuple[int, int, str]] = []
    for y in range(maze.height):
        for x in range(maze.width):
            v = maze.cell(x, y).walls
            if x + 1 < maze.width:
                if bool(v & MOVES["E"][2]) != bool(
                        maze.cell(x + 1, y).walls & MOVES["E"][3]):
                    bad.append((x, y, "E"))
            if y + 1 < maze.height:
                if bool(v & MOVES["S"][2]) != bool(
                        maze.cell(x, y + 1).walls & MOVES["S"][3]):
                    bad.append((x, y, "S"))
    return bad


def open_border(maze: Maze) -> list[tuple[int, int, str]]:
    """Border walls that are not closed."""
    bad: list[tuple[int, int, str]] = []
    for x in range(maze.width):
        if not maze.cell(x, 0).walls & MOVES["N"][2]:
            bad.append((x, 0, "N"))
        if not maze.cell(x, maze.height - 1).walls & MOVES["S"][2]:
            bad.append((x, maze.height - 1, "S"))
    for y in range(maze.height):
        if not maze.cell(0, y).walls & MOVES["W"][2]:
            bad.append((0, y, "W"))
        if not maze.cell(maze.width - 1, y).walls & MOVES["E"][2]:
            bad.append((maze.width - 1, y, "E"))
    return bad


def closed_cells(maze: Maze) -> set[tuple[int, int]]:
    """Cells whose four walls are all closed (digit F)."""
    return {
        (x, y)
        for y in range(maze.height)
        for x in range(maze.width)
        if maze.cell(x, y).walls == 15
    }


def reachable(maze: Maze) -> set[tuple[int, int]]:
    """Cells reachable from the entry (the maze graph component)."""
    return set(bfs(maze).keys())


def isolated_cells(maze: Maze, closed: set[tuple[int, int]]) -> list[tuple[int, int]]:
    """Traversable cells that are not reachable from the entry."""
    reach = reachable(maze)
    return sorted(
        (x, y)
        for y in range(maze.height)
        for x in range(maze.width)
        if maze.is_traversable(x, y) and (x, y) not in reach
    )


def open_3x3(maze: Maze) -> list[tuple[int, int]]:
    """Top-left corners of fully open 3x3 blocks."""
    out: list[tuple[int, int]] = []
    for y0 in range(maze.height - 2):
        for x0 in range(maze.width - 2):
            all_e = all(
                maze.passage(x, y, "E")
                for y in range(y0, y0 + 3) for x in range(x0, x0 + 2)
            )
            all_s = all(
                maze.passage(x, y, "S")
                for y in range(y0, y0 + 2) for x in range(x0, x0 + 3)
            )
            if all_e and all_s:
                out.append((x0, y0))
    return out


def cycle_rank(maze: Maze) -> int:
    """Number of independent loops in the reachable part (0 = perfect)."""
    reach = reachable(maze)
    edges = 0
    for y in range(maze.height):
        for x in range(maze.width):
            if (x, y) in reach:
                for letter in ("E", "S"):
                    if maze.passage(x, y, letter):
                        edges += 1
    return edges - len(reach) + 1


def dead_ends(maze: Maze) -> list[tuple[int, int]]:
    """Reachable cells with exactly one open passage."""
    reach = reachable(maze)
    return sorted(
        (x, y) for (x, y) in reach if maze.open_degree(x, y) == 1
    )


def corners(maze: Maze) -> list[tuple[int, int]]:
    """The four corner cells."""
    return [
        (0, 0),
        (maze.width - 1, 0),
        (0, maze.height - 1),
        (maze.width - 1, maze.height - 1),
    ]


def centres(maze: Maze) -> list[tuple[int, int]]:
    """The up-to-four central cells of the grid."""
    xs = sorted({(maze.width - 1) // 2, maze.width // 2})
    ys = sorted({(maze.height - 1) // 2, maze.height // 2})
    return [(x, y) for y in ys for x in xs]


def validate(maze: Maze, perfect: bool) -> list[str]:
    """Return a list of rule violations (empty means valid)."""
    problems: list[str] = []
    if wall_mismatch(maze):
        problems.append("walls are not coherent between neighbours")
    if open_border(maze):
        problems.append("the outer border is not fully closed")
    closed = closed_cells(maze)
    if maze.entry in closed or maze.exit in closed:
        problems.append("entry or exit is fully closed")
    if isolated_cells(maze, closed):
        problems.append("there are isolated traversable cells")
    if open_3x3(maze):
        problems.append("there is a fully open 3x3 area")
    if perfect:
        if cycle_rank(maze) > 0:
            problems.append("perfect maze contains a loop")
    else:
        if cycle_rank(maze) < 2 and maze.width * maze.height >= 60:
            problems.append("non-perfect board has less than two loops")
        for corner in corners(maze):
            if not maze.is_traversable(*corner) or corner in closed:
                problems.append("a corner is not an open corridor")
        if not any(
            maze.is_traversable(*c) and c not in closed for c in centres(maze)
        ):
            problems.append("the centre is not an open corridor")
    return problems
