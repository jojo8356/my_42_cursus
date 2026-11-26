"""mazegen: a reusable maze generator package.

The package knows nothing about the configuration file, the terminal or
the command line. It can be imported and used in another project.

    from mazegen import MazeGenerator

    gen = MazeGenerator(15, 11, (0, 0), (14, 10), perfect=True, seed=42)
    gen.generate()
    print(gen.walls)      # grid of wall values
    print(gen.solve())    # shortest path as N/E/S/W letters
"""

from .errors import (
    ConfigError,
    MazeError,
    OutputError,
    SolveError,
    ValidationError,
)
from .generator import MazeGenerator
from .model import Maze
from .output import to_text, write_output
from .parser import Config, parse_file, parse_text
from .solver import shortest_path
from .validator import validate

__all__ = [
    "MazeGenerator",
    "Maze",
    "Config",
    "parse_file",
    "parse_text",
    "shortest_path",
    "to_text",
    "write_output",
    "validate",
    "MazeError",
    "ConfigError",
    "ValidationError",
    "OutputError",
    "SolveError",
]
