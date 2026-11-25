"""Breadth-first shortest-path solver."""

from __future__ import annotations

from collections import deque

from .errors import SolveError
from .model import Maze


def bfs(maze: Maze) -> dict[tuple[int, int], tuple[tuple[int, int], str]]:
    """BFS from the entry. Returns {cell: (previous_cell, move)}."""
    start = maze.entry
    prev: dict[tuple[int, int], tuple[tuple[int, int], str]] = {start: (start, "")}
    queue: deque[tuple[int, int]] = deque([start])
    while queue:
        x, y = queue.popleft()
        for nx, ny, letter in maze.neighbours(x, y):
            if (nx, ny) not in prev:
                prev[(nx, ny)] = ((x, y), letter)
                queue.append((nx, ny))
    return prev


def shortest_path(maze: Maze) -> str:
    """The shortest path from the entry to the exit, as N/E/S/W letters."""
    prev = bfs(maze)
    if maze.exit not in prev:
        raise SolveError("no path between the entry and the exit")
    moves: list[str] = []
    current = maze.exit
    while current != maze.entry:
        _previous, letter = prev[current]
        moves.append(letter)
        current = prev[current][0]
    return "".join(reversed(moves))


def is_shortest(maze: Maze, path: str) -> bool:
    """True when `path` is a valid shortest path from entry to exit."""
    best = shortest_path(maze)
    return maze_walk(maze, path)[0] and len(path) == len(best)


def maze_walk(maze: Maze, path: str) -> tuple[bool, str, list[tuple[int, int]]]:
    """Replay `path` from the entry. Returns (ok, why, visited)."""
    from .model import MOVES

    x, y = maze.entry
    seen = [(x, y)]
    for index, letter in enumerate(path):
        if letter not in MOVES:
            return False, f"move {index + 1} is {letter!r}", seen
        dx, dy, _bit, _opp = MOVES[letter]
        nx, ny = x + dx, y + dy
        if not maze.passage(x, y, letter):
            return False, f"move {index + 1} ({letter}) hits a closed wall", seen
        x, y = nx, ny
        seen.append((x, y))
    if (x, y) != maze.exit:
        return False, "the path does not end at the exit", seen
    return True, "", seen
