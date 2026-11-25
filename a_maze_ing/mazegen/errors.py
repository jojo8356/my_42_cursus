"""Exceptions used across the mazegen package."""


class MazeError(Exception):
    """Base class for every maze-related error."""


class ConfigError(MazeError):
    """The configuration file is invalid."""


class ValidationError(MazeError):
    """A generated maze does not respect the subject rules."""


class OutputError(MazeError):
    """The output file could not be written."""


class SolveError(MazeError):
    """No path could be found between the entry and the exit."""
