#!/usr/bin/env python3
"""A-Maze-ing: generate, solve and display a maze from a config file.

Usage:
    python3 a_maze_ing.py config.txt
"""

from __future__ import annotations

import random
import sys

from mazegen import MazeGenerator, write_output
from mazegen.errors import MazeError
from mazegen.menu import interactive_session
from mazegen.parser import Config, parse_file


def _error(message: str) -> None:
    """Print a clear error message and exit with status 1."""
    print(f"Error: {message}", file=sys.stderr)
    raise SystemExit(1)


def _build(config: Config, seed: int | None) -> MazeGenerator:
    """Create a generator and write its output file."""
    generator = MazeGenerator(
        config.width,
        config.height,
        config.entry,
        config.exit,
        perfect=config.perfect,
        seed=seed,
        algorithm=config.algorithm,
    )
    if generator.pattern_omitted:
        print("Warning: the 42 pattern could not be placed "
              "(the maze is too small).")
    assert generator.maze is not None
    write_output(generator.maze, generator.path, config.output_file)
    return generator


def main(argv: list[str]) -> int:
    """Run the CLI: parse the config, generate, write and display."""
    if len(argv) != 1:
        _error("usage: python3 a_maze_ing.py <config.txt>")

    try:
        config = parse_file(argv[0])
    except MazeError as exc:
        _error(str(exc))
        return 1

    try:
        generator = _build(config, config.seed)
    except MazeError as exc:
        _error(str(exc))
        return 1

    def regenerate() -> MazeGenerator:
        """Build a new maze with a fresh seed and write its output."""
        new_seed = random.randrange(0, 2 ** 31 - 1)
        return _build(config, new_seed)

    interactive_session(generator, regenerate)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except MazeError as exc:
        _error(str(exc))
