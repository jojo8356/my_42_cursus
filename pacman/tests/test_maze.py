"""Tests for the maze generator and maze model."""
import pytest

from maze import Maze, Cell, NORTH, EAST, SOUTH, WEST
from model import WALL_TO_DIRECTION


def test_maze_dimensions():
    m = Maze(10, 12, seed=0)
    assert m.width == 10
    assert m.height == 12
    assert len(m.grid) == 12
    assert len(m.grid[0]) == 10


def test_maze_seed_reproducibility():
    m1 = Maze(8, 8, seed=42)
    m2 = Maze(8, 8, seed=42)
    m3 = Maze(8, 8, seed=1)
    for r in range(8):
        for c in range(8):
            assert m1.get(r, c).walls == m2.get(r, c).walls
    # A different seed generally produces a different layout.
    different = any(
        m1.get(r, c).walls != m3.get(r, c).walls
        for r in range(8) for c in range(8)
    )
    assert different


def test_maze_invalid_dimensions():
    with pytest.raises(ValueError):
        Maze(0, 10)
    with pytest.raises(ValueError):
        Maze(10, 0)


def test_maze_start_cell_has_open_path():
    # The start cell (0,0) must have at least one passable direction.
    m = Maze(5, 5, seed=7)
    cell = m.get(0, 0)
    assert any(
        m.is_passable(0, 0, d) for d in (NORTH, EAST, SOUTH, WEST)
    )


def test_maze_is_connected():
    # Every cell must be reachable from (0,0) (perfect-maze property).
    m = Maze(6, 6, seed=3, perfect=True)
    from ghosts import bfs_distances
    distances = bfs_distances(m, 0, 0)
    assert len(distances) == m.width * m.height


def test_maze_non_perfect_has_loops():
    # A non-perfect maze has at least one loop (a cell reachable by more
    # than one path). We detect it by checking that removing walls created
    # extra connections: count of open passages > width*height - 1.
    m = Maze(12, 12, seed=5, perfect=False)
    open_passages = 0
    for r in range(m.height):
        for c in range(m.width):
            for direction in (NORTH, EAST):
                dr, dc = (-1, 0) if direction == NORTH else (0, 1)
                nr, nc = r + dr, c + dc
                if 0 <= nr < m.height and 0 <= nc < m.width:
                    if not m.get(r, c).has_wall(direction):
                        open_passages += 1
    # Perfect maze has exactly width*height - 1 passages; non-perfect has more.
    assert open_passages > m.width * m.height - 1


def test_maze_tunnel_west_east():
    m = Maze(5, 5, seed=0)
    # From the left border, moving west should wrap to the right border.
    assert m.is_passable(2, 0, WEST)
    nr, nc = m.move(2, 0, WEST)
    assert (nr, nc) == (2, m.width - 1)
    # From the right border, moving east wraps to the left.
    assert m.is_passable(2, m.width - 1, EAST)
    nr, nc = m.move(2, m.width - 1, EAST)
    assert (nr, nc) == (2, 0)


def test_maze_tunnel_north_south_blocked():
    m = Maze(5, 5, seed=0)
    # There is no vertical tunnel: moving north from the top row is blocked
    # (unless the cell has no north wall, which the border keeps).
    assert not m.is_passable(0, 2, NORTH)


def test_maze_neighbors_returns_passable():
    m = Maze(5, 5, seed=9)
    neighbors = m.neighbors(2, 2)
    assert isinstance(neighbors, list)
    for direction, nr, nc in neighbors:
        assert m.in_bounds(nr, nc)
        # Each reported neighbour must be reachable from the source cell.
        assert m.is_passable(2, 2, direction)
        # The reverse move from the neighbour back is also passable.
        d = WALL_TO_DIRECTION[direction]
        assert m.is_passable(nr, nc, d.opposite)


def test_cell_wall_flags():
    cell = Cell()
    assert cell.has_wall(NORTH)
    cell.remove_wall(NORTH)
    assert not cell.has_wall(NORTH)
    cell.add_wall(NORTH)
    assert cell.has_wall(NORTH)
    assert NORTH in cell.walls_present()


def test_pacgums_positions_excludes_border():
    m = Maze(6, 6, seed=0)
    positions = m.pacgums_positions
    assert all(
        0 < r < m.height - 1 and 0 < c < m.width - 1
        for r, c in positions
    )
    # Should cover the interior.
    assert len(positions) == (m.height - 2) * (m.width - 2)
