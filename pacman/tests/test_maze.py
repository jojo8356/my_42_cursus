"""Tests for maze generation, movement and wall semantics."""
import pytest

from ghosts import bfs_distances
from maze import Cell, EAST, NORTH, SOUTH, WEST, Maze
from model import WALL_TO_DIRECTION


def test_maze_dimensions() -> None:
    maze = Maze(10, 12, seed=0)
    assert maze.width == 10
    assert maze.height == 12
    assert len(maze.grid) == 12
    assert len(maze.grid[0]) == 10


def test_maze_seed_reproducibility() -> None:
    first = Maze(8, 8, seed=42)
    second = Maze(8, 8, seed=42)
    different_seed = Maze(8, 8, seed=1)
    for row in range(8):
        for col in range(8):
            assert first.get(row, col).walls == second.get(row, col).walls
    assert any(
        first.get(row, col).walls != different_seed.get(row, col).walls
        for row in range(8)
        for col in range(8)
    )


def test_maze_invalid_dimensions() -> None:
    with pytest.raises(ValueError):
        Maze(0, 10)
    with pytest.raises(ValueError):
        Maze(10, 0)
    with pytest.raises(ValueError):
        Maze(True, 10)
    with pytest.raises(ValueError):
        Maze(3.5, 10)  # type: ignore[arg-type]


def test_maze_start_cell_has_open_path() -> None:
    maze = Maze(5, 5, seed=7)
    assert any(
        maze.is_passable(0, 0, direction)
        for direction in (NORTH, EAST, SOUTH, WEST)
    )


def test_perfect_maze_is_connected() -> None:
    maze = Maze(6, 6, seed=3, perfect=True)
    assert len(bfs_distances(maze, 0, 0)) == maze.width * maze.height


def test_non_perfect_maze_has_loops() -> None:
    maze = Maze(12, 12, seed=5, perfect=False)
    open_passages = 0
    for row in range(maze.height):
        for col in range(maze.width):
            for direction in (NORTH, EAST):
                dr, dc = (-1, 0) if direction == NORTH else (0, 1)
                next_row, next_col = row + dr, col + dc
                if (
                    0 <= next_row < maze.height
                    and 0 <= next_col < maze.width
                    and not maze.get(row, col).has_wall(direction)
                ):
                    open_passages += 1
    assert open_passages > maze.width * maze.height - 1


def test_maze_tunnel_west_east() -> None:
    maze = Maze(5, 5, seed=0)
    assert maze.is_passable(2, 0, WEST)
    assert maze.move(2, 0, WEST) == (2, maze.width - 1)
    assert maze.is_passable(2, maze.width - 1, EAST)
    assert maze.move(2, maze.width - 1, EAST) == (2, 0)


def test_maze_vertical_border_is_blocked() -> None:
    maze = Maze(5, 5, seed=0)
    assert not maze.is_passable(0, 2, NORTH)
    assert not maze.is_passable(-1, 2, NORTH)


def test_maze_rejects_invalid_directions() -> None:
    maze = Maze(5, 5, seed=0)
    with pytest.raises(ValueError):
        maze.is_passable(2, 2, 0)


def test_maze_to_grid_returns_the_raw_grid() -> None:
    maze = Maze(5, 5, seed=0)
    assert maze.to_grid() is maze.grid


def test_maze_neighbors_returns_mutually_passable_cells() -> None:
    maze = Maze(5, 5, seed=9)
    neighbors = maze.neighbors(2, 2)
    assert isinstance(neighbors, list)
    for wall, next_row, next_col in neighbors:
        direction = WALL_TO_DIRECTION[wall]
        assert maze.in_bounds(next_row, next_col)
        assert maze.is_passable(2, 2, direction)
        assert maze.is_passable(next_row, next_col, direction.opposite)


def test_cell_wall_flags() -> None:
    cell = Cell()
    assert cell.has_wall(NORTH)
    cell.remove_wall(NORTH)
    assert not cell.has_wall(NORTH)
    cell.add_wall(NORTH)
    assert cell.has_wall(NORTH)
    assert NORTH in cell.walls_present()


def test_pacgum_positions_exclude_border() -> None:
    maze = Maze(6, 6, seed=0)
    positions = maze.pacgums_positions
    assert all(
        0 < row < maze.height - 1 and 0 < col < maze.width - 1
        for row, col in positions
    )
    assert len(positions) == (maze.height - 2) * (maze.width - 2)
