"""Tests for ghost pathfinding, personality targets and movement modes."""
import random

import pytest

from ghosts import (
    CORNERS,
    bfs_distances,
    choose_direction,
    compute_target,
    direction_between,
    shortest_path,
)
from maze import EAST, NORTH, SOUTH, WEST, Maze
from model import (
    Direction,
    Ghost,
    GhostKind,
    GhostMode,
    Position,
    WALL_TO_DIRECTION,
)


@pytest.fixture
def maze() -> Maze:
    return Maze(10, 10, seed=42, perfect=False)


def test_bfs_distances(maze: Maze) -> None:
    distances = bfs_distances(maze, 0, 0)
    assert distances[(0, 0)] == 0
    assert len(distances) == maze.width * maze.height


def test_bfs_distances_are_non_negative(maze: Maze) -> None:
    distances = bfs_distances(maze, 0, 0)
    assert all(distance >= 0 for distance in distances.values())


def test_shortest_path(maze: Maze) -> None:
    assert shortest_path(maze, 0, 0, 0, 0) == [(0, 0)]
    path = shortest_path(maze, 0, 0, maze.height - 1, maze.width - 1)
    assert path[0] == (0, 0)
    assert path[-1] == (maze.height - 1, maze.width - 1)
    assert len(path) - 1 == bfs_distances(maze, 0, 0)[
        (maze.height - 1, maze.width - 1)
    ]


def test_shortest_path_unreachable() -> None:
    isolated = Maze(3, 3, seed=0, perfect=True)
    for row in isolated.grid:
        for cell in row:
            for wall in (NORTH, EAST, SOUTH, WEST):
                cell.add_wall(wall)

    assert shortest_path(isolated, 0, 0, 2, 2) == []
    assert shortest_path(isolated, 1, 1, 1, 1) == [(1, 1)]


def test_direction_between(maze: Maze) -> None:
    assert direction_between(maze, 5, 5, (4, 5)) == Direction.NORTH
    assert direction_between(maze, 5, 5, (5, 6)) == Direction.EAST
    assert direction_between(maze, 5, 5, (6, 5)) == Direction.SOUTH
    assert direction_between(maze, 5, 5, (5, 4)) == Direction.WEST


def test_direction_between_tunnel(maze: Maze) -> None:
    assert direction_between(maze, 4, 0, (4, maze.width - 1)) == Direction.WEST
    assert direction_between(maze, 4, maze.width - 1, (4, 0)) == Direction.EAST


def test_chase_targets_are_specific_to_each_ghost() -> None:
    maze = Maze(20, 20, seed=12, perfect=False)
    pacman = (10, 10)
    direction = Direction.EAST
    ghosts = {
        kind: Ghost(kind, Position(2, 2))
        for kind in GhostKind
    }
    ghosts[GhostKind.CLYDE].position = Position(11, 10)

    targets = {
        kind: compute_target(ghost, maze, *pacman, direction)
        for kind, ghost in ghosts.items()
    }

    assert targets[GhostKind.BLINKY] == (10, 10)
    assert targets[GhostKind.PINKY] == (10, 14)
    assert targets[GhostKind.INKY] == (18, 19)
    assert targets[GhostKind.CLYDE] == (19, 0)
    assert len(set(targets.values())) == 4


def test_clyde_chases_when_far_and_scatters_when_close(maze: Maze) -> None:
    far_clyde = Ghost(GhostKind.CLYDE, Position(0, 0))
    near_clyde = Ghost(GhostKind.CLYDE, Position(5, 5))

    assert compute_target(
        far_clyde,
        maze,
        9,
        9,
        Direction.EAST,
    ) == (9, 9)
    assert compute_target(
        near_clyde,
        maze,
        5,
        6,
        Direction.EAST,
    ) == (maze.height - 1, 0)


def test_scatter_targets_are_distinct_corners(maze: Maze) -> None:
    expected = {
        GhostKind.BLINKY: (0, 0),
        GhostKind.PINKY: (0, maze.width - 1),
        GhostKind.INKY: (maze.height - 1, maze.width - 1),
        GhostKind.CLYDE: (maze.height - 1, 0),
    }
    for kind, target in expected.items():
        ghost = Ghost(kind, Position(5, 5))
        ghost.mode = GhostMode.SCATTER
        assert compute_target(ghost, maze, 0, 0, Direction.NORTH) == target


def test_compute_target_frightened_is_deterministic(maze: Maze) -> None:
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0))
    ghost.mode = GhostMode.FRIGHTENED
    ghost._rng = random.Random(1)

    first_target = compute_target(ghost, maze, 0, 0, Direction.NORTH)
    ghost._rng = random.Random(1)
    second_target = compute_target(ghost, maze, 0, 0, Direction.NORTH)

    assert first_target == second_target
    assert 0 <= first_target[0] < maze.height
    assert 0 <= first_target[1] < maze.width


def test_compute_target_dead_home(maze: Maze) -> None:
    ghost = Ghost(GhostKind.CLYDE, Position(0, 0))
    ghost.mode = GhostMode.DEAD
    assert compute_target(ghost, maze, 0, 0, Direction.NORTH) == (
        maze.height - 1,
        maze.width // 2,
    )


def test_frightened_target_without_rng_uses_maze_center(maze: Maze) -> None:
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0))
    ghost.mode = GhostMode.FRIGHTENED

    assert compute_target(ghost, maze, 0, 0, Direction.NORTH) == (
        maze.height // 2,
        maze.width // 2,
    )


def test_choose_direction_chase_moves_to_a_passable_cell(maze: Maze) -> None:
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH)
    direction = choose_direction(
        ghost,
        maze,
        0,
        9,
        Direction.EAST,
        random.Random(0),
    )
    assert maze.is_passable(0, 0, direction)
    assert maze.move(0, 0, direction) in {
        (row, col) for _wall, row, col in maze.neighbors(0, 0)
    }


def test_choose_direction_does_not_reverse_at_an_intersection(
    maze: Maze,
) -> None:
    ghost = Ghost(GhostKind.BLINKY, Position(5, 5), Direction.NORTH)
    direction = choose_direction(
        ghost,
        maze,
        0,
        0,
        Direction.NORTH,
        random.Random(0),
    )
    options = maze.neighbors(5, 5)
    assert direction != Direction.SOUTH or len(options) == 1


def test_choose_direction_dead_follows_shortest_path_home(maze: Maze) -> None:
    ghost = Ghost(GhostKind.INKY, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.DEAD
    path = shortest_path(maze, 0, 0, maze.height - 1, maze.width // 2)

    direction = choose_direction(
        ghost,
        maze,
        5,
        5,
        Direction.EAST,
        random.Random(0),
    )

    assert direction == direction_between(maze, 0, 0, path[1])


def test_dead_ghost_at_home_keeps_its_direction(maze: Maze) -> None:
    ghost = Ghost(
        GhostKind.BLINKY,
        Position(maze.height - 1, maze.width // 2),
        Direction.WEST,
    )
    ghost.mode = GhostMode.DEAD

    assert choose_direction(
        ghost,
        maze,
        0,
        0,
        Direction.NORTH,
        random.Random(0),
    ) == Direction.WEST


def test_choose_direction_frightened_maximizes_distance(maze: Maze) -> None:
    ghost = Ghost(GhostKind.BLINKY, Position(0, 0), Direction.SOUTH)
    ghost.mode = GhostMode.FRIGHTENED
    pacman = (0, 9)
    distances = bfs_distances(maze, *pacman)
    options = [
        (WALL_TO_DIRECTION[wall], row, col)
        for wall, row, col in maze.neighbors(0, 0)
        if WALL_TO_DIRECTION[wall] != ghost.direction.opposite
    ]

    direction = choose_direction(
        ghost,
        maze,
        *pacman,
        Direction.EAST,
        random.Random(0),
    )
    selected = next(option for option in options if option[0] == direction)
    selected_distance = distances[(selected[1], selected[2])]

    assert selected_distance == max(
        distances[(row, col)] for _direction, row, col in options
    )


def test_choose_direction_keeps_facing_if_fully_trapped() -> None:
    isolated = Maze(3, 3, seed=0, perfect=True)
    for row in isolated.grid:
        for cell in row:
            for wall in (NORTH, EAST, SOUTH, WEST):
                cell.add_wall(wall)
    ghost = Ghost(GhostKind.BLINKY, Position(1, 1), Direction.EAST)

    direction = choose_direction(
        ghost,
        isolated,
        0,
        0,
        Direction.NORTH,
        random.Random(0),
    )

    assert direction == Direction.EAST


def test_choose_direction_reverses_at_a_dead_end() -> None:
    small = Maze(6, 6, seed=0, perfect=True)
    cell = small.get(2, 2)
    for wall in (EAST, SOUTH, WEST):
        cell.add_wall(wall)
    cell.remove_wall(NORTH)
    ghost = Ghost(GhostKind.BLINKY, Position(2, 2), Direction.SOUTH)

    direction = choose_direction(
        ghost,
        small,
        5,
        5,
        Direction.SOUTH,
        random.Random(0),
    )

    assert direction == Direction.NORTH


def test_corners_defined() -> None:
    assert set(CORNERS) == set(GhostKind)
