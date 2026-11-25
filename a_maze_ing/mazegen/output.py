"""Serialize a maze to the subject's hexadecimal output format."""

from __future__ import annotations

from pathlib import Path

from .errors import OutputError
from .model import Maze


def to_hex_lines(maze: Maze) -> list[str]:
    """The grid as one uppercase hexadecimal digit per cell, row by row."""
    lines: list[str] = []
    for y in range(maze.height):
        line = "".join(f"{maze.cell(x, y).walls:X}" for x in range(maze.width))
        lines.append(line)
    return lines


def to_text(maze: Maze, path: str) -> str:
    """The full output file content.

    Grid rows, one empty line, then the entry, the exit and the shortest
    path, every line ending with a newline.
    """
    parts = to_hex_lines(maze)
    parts.append("")
    parts.append(f"{maze.entry[0]},{maze.entry[1]}")
    parts.append(f"{maze.exit[0]},{maze.exit[1]}")
    parts.append(path)
    return "\n".join(parts) + "\n"


def write_output(maze: Maze, path: str, filename: str | Path) -> None:
    """Write the serialized maze to `filename`."""
    target = Path(filename)
    try:
        target.write_text(to_text(maze, path), encoding="utf-8")
    except OSError as exc:
        raise OutputError(f"cannot write the output file {target}: {exc}") from exc
