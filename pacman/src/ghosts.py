"""
Ghost AI for Pacman.

Each ghost moves on the maze grid and picks a direction at intersections.
The behaviour is driven by a **breadth-first search** over the maze (which
handles the tunnel wrap-around) to compute distances and pick the next cell
closest to a target. This mirrors the classic arcade ghost logic:

- **Chase**: each ghost targets a different cell derived from Pac-Man's
  position (Blinky = Pac-Man, Pinky = 4 cells ahead, Inky = mixed, Clyde =
  far-chase / near-scatter).
- **Scatter**: each ghost heads to its own corner.
- **Frightened**: ghosts wander randomly and prefer moves that keep them far
  from Pac-Man; they become edible.
- **Dead**: a ghost returns to its home cell along the shortest path.

All randomness is injected through a ``random.Random`` so behaviour is
deterministic and testable.
"""
import random

from maze import NORTH, EAST, SOUTH, WEST, DIR_VEC
from model import Direction, GhostKind, GhostMode, WALL_TO_DIRECTION

# Corner targets for the scatter mode (top-left, top-right, bottom-right,
# bottom-left) in (row, col).
CORNERS = {
    GhostKind.BLINKY: (0, 0),
    GhostKind.PINKY: (0, -1),       # top-right
    GhostKind.INKY: (-1, -1),       # bottom-right
    GhostKind.CLYDE: (-1, 0),       # bottom-left
}


def _flag_to_direction(flag):
    return WALL_TO_DIRECTION[flag]


def bfs_distances(maze, start_row, start_col):
    """Breadth-first search returning a dict {(row, col): distance}.

    The tunnel wrap-around is respected: a step that exits the left border
    reappears on the right border (and vice versa).
    """
    distances = {}
    queue = [(start_row, start_col)]
    distances[(start_row, start_col)] = 0
    idx = 0
    while idx < len(queue):
        r, c = queue[idx]
        idx += 1
        for direction, nr, nc in maze.neighbors(r, c):
            key = (nr, nc)
            if key not in distances:
                distances[key] = distances[(r, c)] + 1
                queue.append((nr, nc))
    return distances


def shortest_path(maze, start_row, start_col, target_row, target_col):
    """Return the list of (row, col) from start to target (BFS).

    Returns an empty list if the target is unreachable. The first step is
    the direction a ghost should take to begin the path.
    """
    if (start_row, start_col) == (target_row, target_col):
        return [(start_row, start_col)]
    distances = bfs_distances(maze, start_row, start_col)
    if (target_row, target_col) not in distances:
        return []
    # Reconstruct the path greedily by always stepping to a neighbour that
    # is one step closer to the target.
    path = [(target_row, target_col)]
    r, c = target_row, target_col
    while (r, c) != (start_row, start_col):
        current_dist = distances[(r, c)]
        found = False
        for direction, nr, nc in maze.neighbors(r, c):
            if distances.get((nr, nc)) == current_dist - 1:
                path.append((nr, nc))
                r, c = nr, nc
                found = True
                break
        if not found:
            return []
    path.reverse()
    return path


def compute_target(ghost, maze, pacman_row, pacman_col, pacman_direction):
    """Return the target (row, col) for a ghost in its current mode."""
    kind = ghost.kind
    mode = ghost.mode
    h, w = maze.height, maze.width

    if mode == GhostMode.SCATTER:
        corn = CORNERS[kind]
        # Sentinel -1 means the far edge (right / bottom).
        target_row = corn[0] if corn[0] >= 0 else h - 1
        target_col = corn[1] if corn[1] >= 0 else w - 1
        return (max(0, min(h - 1, target_row)),
                max(0, min(w - 1, target_col)))

    if mode == GhostMode.DEAD:
        # Return home: bottom-centre of the maze.
        return (h - 1, w // 2)

    if mode == GhostMode.FRIGHTENED:
        # Wander: target is a random cell (or far from Pac-Man).
        rng = getattr(ghost, "_rng", None)
        if rng is not None:
            return (rng.randrange(h), rng.randrange(w))
        return (h // 2, w // 2)

    # CHASE
    if kind == GhostKind.BLINKY:
        return (pacman_row, pacman_col)

    if kind == GhostKind.PINKY:
        # 4 cells ahead of Pac-Man in its direction.
        dr, dc = pacman_direction.vector
        ahead_r = pacman_row + dr * 4
        ahead_c = pacman_col + dc * 4
        # Clamp into the maze (approximation).
        return (max(0, min(h - 1, ahead_r)), max(0, min(w - 1, ahead_c)))

    if kind == GhostKind.INKY:
        # Impredictable: mix of chase and random. The caller decides via a
        # coin flip; here we return Pac-Man (the caller may override).
        return (pacman_row, pacman_col)

    if kind == GhostKind.CLYDE:
        # If far from Pac-Man, chase; if close, scatter to corner.
        manhattan = abs(ghost.position.row - pacman_row) + abs(ghost.position.col - pacman_col)
        if manhattan >= 8:
            return (pacman_row, pacman_col)
        corn = CORNERS[kind]
        return (max(0, min(h - 1, corn[0])), max(0, min(w - 1, corn[1])))

    return (pacman_row, pacman_col)


def choose_direction(ghost, maze, pacman_row, pacman_col, pacman_direction, rng):
    """Choose the next direction for a ghost at its current cell.

    The ghost never reverses direction unless it is in a dead end or is
    forced to (mode transitions / frightened). Returns a ``Direction``.
    """
    row, col = ghost.position.row, ghost.position.col
    ghost._rng = rng

    # If dead, follow the shortest path home.
    if ghost.mode == GhostMode.DEAD:
        path = shortest_path(maze, row, col, maze.height - 1, maze.width // 2)
        if len(path) >= 2:
            return direction_between(maze, row, col, path[1])
        # If home reached, revive.
        return ghost.direction

    # Determine the target.
    target = compute_target(ghost, maze, pacman_row, pacman_col, pacman_direction)
    target_r, target_c = target

    # Gather passable neighbours (excluding the reverse direction unless no
    # other option).
    options = []
    for flag, nr, nc in maze.neighbors(row, col):
        direction = WALL_TO_DIRECTION[flag]
        if direction == ghost.direction.opposite:
            continue
        options.append((direction, nr, nc))

    if not options:
        # Dead end: reverse.
        if maze.is_passable(row, col, ghost.direction.opposite):
            return ghost.direction.opposite
        return ghost.direction

    if ghost.mode == GhostMode.FRIGHTENED:
        # Prefer the move that maximises distance from Pac-Man, with a
        # random tie-break.
        distances = bfs_distances(maze, pacman_row, pacman_col)

        def score(cell):
            return distances.get(cell, 0)

        options.sort(key=lambda o: (-score(o[1:]), rng.random()))
        return options[0][0]

    # CHASE / SCATTER: pick the neighbour closest to the target.
    distances = bfs_distances(maze, target_r, target_c)

    def score(cell):
        return distances.get(cell, float("inf"))

    options.sort(key=lambda o: (score(o[1:]), rng.random()))
    return options[0][0]


def direction_between(maze, row, col, next_cell):
    """Return the Direction that moves from (row, col) to ``next_cell``."""
    dr = next_cell[0] - row
    dc = next_cell[1] - col
    if (dr, dc) == (-1, 0):
        return Direction.NORTH
    if (dr, dc) == (0, 1):
        return Direction.EAST
    if (dr, dc) == (1, 0):
        return Direction.SOUTH
    if (dr, dc) == (0, -1):
        return Direction.WEST
    # Tunnels can wrap: same row, col jumps between 0 and width-1.
    if dr == 0 and dc != 0:
        return Direction.EAST if dc > 0 else Direction.WEST
    if dc == 0 and dr != 0:
        return Direction.SOUTH if dr > 0 else Direction.NORTH
    raise ValueError(f"No direction from ({row}, {col}) to {next_cell}")
