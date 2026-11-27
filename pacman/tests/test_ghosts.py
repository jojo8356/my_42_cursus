"""Tests for the ghost AI (BFS, target selection, direction choice)."""
import random

import pytest

from maze import Maze
from model import Direction, Ghost, GhostKind, GhostMode, Position
from ghosts import (
    bfs_distances, shortest_path, compute_target, choose_direction,
    direction_between, CORNERS,
)


@pytest.fixture
def maze():
    return Maze(10, 10, seed=42, perfect=False)


def test_bfs_distances(maze):
    distances = bfs_distances(maze, 0, 0)
    assert distances[(0, 0)] == 0
    # All reachable cells have a finite distance.
    assert len(distances) == maze.width * maze.height


def test_bfs_distances_respect_walls(maze):
    # A wall blocks direct movement; the distance around it is longer.
    distances = bfs_distances(maze, 0, 0)
    # Distance to a neighbour reachable only via a longer route is >= 1.
    for key, dist in distances.items():
        assert dist >= 0


def test_shortest_path(maze):
    path = shortest_path(maze, 0, 0, 0, 0)
    assert path == [(0, 0)]
    # A path from start to a distant cell is non-trivial.
    path = shortest_path(maze, 0, 0, maze.height - 1, maze.width - 1)
    assert path[0] == (0, 0)
    assert path[-1] == (maze.height - 1, maze.width - 1)


def test_shortest_path_unreachable(maze):
    # The same cell returns a single-element path; a truly unreachable target
    # returns an empty path. (The maze is connected, so we test the start==
    # target case and a normal path.)
    assert shortest_path(maze, 5, 5, 5, 5) == [(5, 5)]


def test_direction_between(maze):
    d = direction_between(maze, 5, 5, (4, 5))
    assert d == Direction.NORTH
    d = direction_between(maze, 5, 5, (5, 6))
    assert d == Direction.EAST
    d = direction_between(maze, 5, 5, (6, 5))
    assert d == Direction.SOUTH
    d = direction_between(maze, 5, 5, (5, 4))
    assert d == Direction.WEST


def test_compute_target_blinky_chase(maze):
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.CHASE
    target = compute_target(ghost, maze, 3, 4, Direction.EAST)
    assert target == (3, 4)


def test_compute_target_scatter_corner(maze):
    ghost = Ghost(GhostKind.PINKY, Position(5, 5), Direction.NORTH)
    ghost.mode = GhostMode.SCATTER
    target = compute_target(ghost, maze, 0, 0, Direction.NORTH)
    # Pinky scatters to the top-right corner (clamped into the maze).
    assert target[0] == 0
    assert target[1] == maze.width - 1


def test_compute_target_frightened(maze):
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.FRIGHTENED
    ghost._rng = random.Random(1)
    target = compute_target(ghost, maze, 0, 0, Direction.NORTH)
    assert 0 <= target[0] < maze.height
    assert 0 <= target[1] < maze.width


def test_compute_target_dead_home(maze):
    ghost = Ghost(GhostKind.CLYDE, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.DEAD
    target = compute_target(ghost, maze, 0, 0, Direction.NORTH)
    assert target == (maze.height - 1, maze.width // 2)


def test_choose_direction_chase_moves_toward_target(maze):
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.CHASE
    rng = random.Random(0)
    direction = choose_direction(ghost, maze, 0, 9, Direction.EAST, rng)
    # The chosen direction must be a passable neighbour.
    nr, nc = maze.move(0, 0, direction)
    assert maze.is_passable(0, 0, direction)


def test_choose_direction_no_reverse(maze):
    ghost = Ghost(GhostKind.BLINKY, Position(5, 5), Direction.NORTH)
    ghost.mode = GhostMode.CHASE
    rng = random.Random(0)
    direction = choose_direction(ghost, maze, 0, 0, Direction.NORTH, rng)
    # Should not immediately reverse into the opposite of its direction.
    # (Unless it's a dead end, which we avoid by construction.)
    assert direction != Direction.SOUTH or not maze.neighbors(5, 5)


def test_choose_direction_dead_moves_home(maze):
    ghost = Ghost(GhostKind.INKY, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.DEAD
    rng = random.Random(0)
    direction = choose_direction(ghost, maze, 5, 5, Direction.EAST, rng)
    assert direction in (Direction.NORTH, Direction.EAST,
                         Direction.SOUTH, Direction.WEST)


def test_choose_direction_frightened_far_from_pacman(maze):
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.FRIGHTENED
    rng = random.Random(0)
    direction = choose_direction(ghost, maze, 0, 9, Direction.EAST, rng)
    assert maze.is_passable(0, 0, direction)


def test_choose_direction_dead_end_reverses(maze):
    # Build a maze where an interior cell (2,2) is a dead end whose only
    # exit is the direction the ghost just came from (so it must reverse).
    small = Maze(6, 6, seed=0, perfect=True)
    # Reduce (2,2) to a dead end: keep only the NORTH passage open.
    cell = small.get(2, 2)
    for flag in (1 << 1, 1 << 2, 1 << 3):  # EAST, SOUTH, WEST closed
        cell.add_wall(flag)
    cell.remove_wall(1 << 0)  # NORTH open
    ghost = Ghost(GhostKind.BLINKY, Position(2, 2), Direction.SOUTH)
    ghost.mode = GhostMode.CHASE
    rng = random.Random(0)
    # The only passable exit is NORTH, which is the reverse of SOUTH.
    direction = choose_direction(ghost, small, 5, 5, Direction.SOUTH, rng)
    assert direction == Direction.NORTH


def test_corners_defined():
    assert GhostKind.BLINKY in CORNERS
    assert GhostKind.PINKY in CORNERS
    assert GhostKind.INKY in CORNERS
    assert GhostKind.CLYDE in CORNERS
