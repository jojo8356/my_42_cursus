"""ASCII rendering of a maze for the terminal."""

from __future__ import annotations

from .model import MOVES, Maze

PALETTE = [
    {"wall": "\033[38;5;240m", "open": "\033[38;5;231m", "reset": "\033[0m"},
    {"wall": "\033[38;5;231m", "open": "\033[38;5;240m", "reset": "\033[0m"},
    {"wall": "\033[38;5;220m", "open": "\033[38;5;235m", "reset": "\033[0m"},
    {"wall": "\033[38;5;235m", "open": "\033[38;5;220m", "reset": "\033[0m"},
]


def render(maze: Maze, show_path: bool = False, path: str = "",
           palette: dict[str, str] | None = None) -> str:
    """Render the maze as ASCII art, optionally with the path overlay."""
    colours = palette or PALETTE[0]
    visited: set[tuple[int, int]] = set()
    if show_path:
        x, y = maze.entry
        visited.add((x, y))
        for letter in path:
            dx, dy = MOVES[letter][0], MOVES[letter][1]
            x, y = x + dx, y + dy
            visited.add((x, y))

    lines: list[str] = []
    for y in range(maze.height):
        top = "+"
        for x in range(maze.width):
            top += "--" if maze.cell(x, y).walls & MOVES["N"][2] else "  "
        top += "+"
        lines.append(top)
        mid = ""
        for x in range(maze.width):
            left = "|" if maze.cell(x, y).walls & MOVES["W"][2] else " "
            body = colours["wall"] + "##" + colours["reset"]
            if (x, y) in visited:
                body = colours["open"] + "  " + colours["reset"]
            if (x, y) == maze.entry:
                body = colours["open"] + "E " + colours["reset"]
            elif (x, y) == maze.exit:
                body = colours["open"] + "X " + colours["reset"]
            mid += left + body
        mid += "|" if maze.cell(maze.width - 1, y).walls & MOVES["E"][2] else " "
        lines.append(mid)
    bottom = "+"
    for x in range(maze.width):
        bottom += "--" if maze.cell(x, maze.height - 1).walls & MOVES["S"][2] else "  "
    bottom += "+"
    lines.append(bottom)
    return "\n".join(lines)
