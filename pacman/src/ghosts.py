"""
Ghost AI for Pacman.

Each ghost moves on the maze grid and picks a direction at intersections.
Breadth-first search computes distances through the maze, including its
left/right tunnel. Chase and scatter targets are specific to each ghost;
frightened ghosts favour moves away from Pac-Man, while dead ghosts return
home along the shortest path.

Randomness is injected through ``random.Random`` so behaviour is deterministic
and testable.
"""
from __future__ import annotations

import random

from maze import Maze
from model import Direction, Ghost, GhostKind, GhostMode, WALL_TO_DIRECTION

Coordinate = tuple[int, int]

# Scatter targets in (row, column); -1 means the far edge of the maze.
CORNERS: dict[GhostKind, Coordinate] = {
    GhostKind.BLINKY: (0, 0),
    GhostKind.PINKY: (0, -1),
    GhostKind.INKY: (-1, -1),
    GhostKind.CLYDE: (-1, 0),
}


def bfs_distances(
    maze: Maze,
    start_row: int,
    start_col: int,
) -> dict[Coordinate, int]:
    """Return shortest distances from a cell using breadth-first search."""
    distances = {(start_row, start_col): 0}
    queue = [(start_row, start_col)]
    index = 0
    while index < len(queue):
        row, col = queue[index]
        index += 1
        for _direction, next_row, next_col in maze.neighbors(row, col):
            next_cell = (next_row, next_col)
            if next_cell not in distances:
                distances[next_cell] = distances[(row, col)] + 1
                queue.append(next_cell)
    return distances


def shortest_path(
    maze: Maze,
    start_row: int,
    start_col: int,
    target_row: int,
    target_col: int,
) -> list[Coordinate]:
    """Return a path to the target, or empty if it cannot be reached."""
    start = (start_row, start_col)
    target = (target_row, target_col)
    if start == target:
        return [start]
    distances = bfs_distances(maze, start_row, start_col)
    if target not in distances:
        return []

    path = [target]
    row, col = target
    while (row, col) != start:
        current_distance = distances[(row, col)]
        for _direction, next_row, next_col in maze.neighbors(row, col):
            if distances.get((next_row, next_col)) == current_distance - 1:
                path.append((next_row, next_col))
                row, col = next_row, next_col
                break
        else:
            return []
    path.reverse()
    return path


def _clamp_target(maze: Maze, row: int, col: int) -> Coordinate:
    return (
        max(0, min(maze.height - 1, row)),
        max(0, min(maze.width - 1, col)),
    )


def _corner_target(ghost: Ghost, maze: Maze) -> Coordinate:
    """Return a ghost's actual corner, resolving -1 as the far edge."""
    row, col = CORNERS[ghost.kind]
    if row < 0:
        row = maze.height - 1
    if col < 0:
        col = maze.width - 1
    return _clamp_target(maze, row, col)


def compute_target(
    ghost: Ghost,
    maze: Maze,
    pacman_row: int,
    pacman_col: int,
    pacman_direction: Direction,
) -> Coordinate:
    """Return the target cell for a ghost in its current mode."""
    if ghost.mode == GhostMode.SCATTER:
        return _corner_target(ghost, maze)

    if ghost.mode == GhostMode.DEAD:
        return maze.height - 1, maze.width // 2

    if ghost.mode == GhostMode.FRIGHTENED:
        if ghost._rng is not None:
            return (
                ghost._rng.randrange(maze.height),
                ghost._rng.randrange(maze.width),
            )
        return maze.height // 2, maze.width // 2

    if ghost.kind == GhostKind.BLINKY:
        return pacman_row, pacman_col

    if ghost.kind == GhostKind.PINKY:
        dr, dc = pacman_direction.vector
        return _clamp_target(
            maze,
            pacman_row + dr * 4,
            pacman_col + dc * 4,
        )

    if ghost.kind == GhostKind.INKY:
        # Aim beyond a point two cells ahead of Pac-Man, reflected across Inky.
        dr, dc = pacman_direction.vector
        ahead_row = pacman_row + dr * 2
        ahead_col = pacman_col + dc * 2
        return _clamp_target(
            maze,
            2 * ahead_row - ghost.position.row,
            2 * ahead_col - ghost.position.col,
        )

    if ghost.kind == GhostKind.CLYDE:
        manhattan = (
            abs(ghost.position.row - pacman_row)
            + abs(ghost.position.col - pacman_col)
        )
        if manhattan >= 8:
            return pacman_row, pacman_col
        return _corner_target(ghost, maze)

    return pacman_row, pacman_col


def choose_direction(
    ghost: Ghost,
    maze: Maze,
    pacman_row: int,
    pacman_col: int,
    pacman_direction: Direction,
    rng: random.Random,
) -> Direction:
    """Choose the next direction for a ghost at its current cell.

    The ghost avoids reversing direction unless it is in a dead end or is
    frightened. The returned direction is always a cardinal direction.
    """
    row, col = ghost.position.row, ghost.position.col
    ghost._rng = rng

    if ghost.mode == GhostMode.DEAD:
        path = shortest_path(
            maze,
            row,
            col,
            maze.height - 1,
            maze.width // 2,
        )
        if len(path) >= 2:
            return direction_between(maze, row, col, path[1])
        return ghost.direction

    target_row, target_col = compute_target(
        ghost,
        maze,
        pacman_row,
        pacman_col,
        pacman_direction,
    )
    options: list[tuple[Direction, int, int]] = []
    for flag, next_row, next_col in maze.neighbors(row, col):
        direction = WALL_TO_DIRECTION[flag]
        if direction != ghost.direction.opposite:
            options.append((direction, next_row, next_col))

    if not options:
        if maze.is_passable(row, col, ghost.direction.opposite):
            return ghost.direction.opposite
        return ghost.direction

    if ghost.mode == GhostMode.FRIGHTENED:
        distances = bfs_distances(maze, pacman_row, pacman_col)
        options.sort(
            key=lambda option: (
                -distances.get((option[1], option[2]), 0),
                rng.random(),
            )
        )
        return options[0][0]

    distances = bfs_distances(maze, target_row, target_col)
    options.sort(
        key=lambda option: (
            distances.get((option[1], option[2]), float("inf")),
            rng.random(),
        )
    )
    return options[0][0]


def direction_between(
    maze: Maze,
    row: int,
    col: int,
    next_cell: Coordinate,
) -> Direction:
    """Return the direction that moves from a cell to its neighbour."""
    delta_row = next_cell[0] - row
    delta_col = next_cell[1] - col
    if (delta_row, delta_col) == (-1, 0):
        return Direction.NORTH
    if (delta_row, delta_col) == (0, 1):
        return Direction.EAST
    if (delta_row, delta_col) == (1, 0):
        return Direction.SOUTH
    if (delta_row, delta_col) == (0, -1):
        return Direction.WEST
    if delta_row == 0 and delta_col != 0:
        if col == 0 and next_cell[1] == maze.width - 1:
            return Direction.WEST
        if col == maze.width - 1 and next_cell[1] == 0:
            return Direction.EAST
        return Direction.EAST if delta_col > 0 else Direction.WEST
    if delta_col == 0 and delta_row != 0:
        return Direction.SOUTH if delta_row > 0 else Direction.NORTH
    raise ValueError(f"No direction from ({row}, {col}) to {next_cell}")
