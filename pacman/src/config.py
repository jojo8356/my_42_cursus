"""
Configuration parsing for Pacman.

The game reads a JSON config file with game-wide settings and a ``levels``
array. This module defines ``GameConfig`` / ``LevelConfig`` data classes and a
robust loader that validates values and falls back to defaults for invalid or
missing settings. Full-line comments beginning with ``#`` are ignored.
"""
import json
import os
from dataclasses import dataclass, field


@dataclass
class LevelConfig:
    """A single level configuration."""
    name: str = "level"
    width: int = 10
    height: int = 10
    pacgums: int = 5
    seed: int = 0
    time: int = 90

    @classmethod
    def from_dict(cls, data):
        if not isinstance(data, dict):
            return cls()
        return cls(
            name=str(data.get("name", "level")),
            width=cls._int(data.get("width"), 10, 1, 100),
            height=cls._int(data.get("height"), 10, 1, 100),
            pacgums=cls._int(data.get("pacgums"), 5, 0, 10000),
            seed=cls._int(data.get("seed"), 0, -10**9, 10**9),
            time=cls._int(data.get("time"), 90, 1, 100000),
        )

    @staticmethod
    def _int(value, default, low, high):
        try:
            v = int(value)
        except (TypeError, ValueError):
            return default
        if v < low or v > high:
            return default
        return v


@dataclass
class GameConfig:
    """Game-wide configuration with defaults."""
    highscore_filename: str = "highscores.json"
    lives: int = 3
    screen_width: int = 1000
    screen_height: int = 1000
    level_max_time: int = 90
    points_per_pacgum: int = 10
    points_per_super_pacgum: int = 50
    points_per_ghost: int = 200
    levels: list = field(default_factory=lambda: GameConfig.default_levels())

    @classmethod
    def from_dict(cls, data):
        if not isinstance(data, dict):
            return cls()
        cfg = cls(
            highscore_filename=str(data.get("highscore_filename", "highscores.json")),
            lives=cls._int(data.get("lives"), 3, 1, 99),
            screen_width=cls._int(data.get("screen_width"), 1000, 100, 10000),
            screen_height=cls._int(data.get("screen_height"), 1000, 100, 10000),
            level_max_time=cls._int(data.get("level_max_time"), 90, 1, 100000),
            points_per_pacgum=cls._int(data.get("points_per_pacgum"), 10, 0, 100000),
            points_per_super_pacgum=cls._int(data.get("points_per_super_pacgum"), 50, 0, 100000),
            points_per_ghost=cls._int(data.get("points_per_ghost"), 200, 0, 100000),
        )
        raw_levels = data.get("levels")
        if isinstance(raw_levels, list) and raw_levels:
            cfg.levels = [LevelConfig.from_dict(item) for item in raw_levels]
        else:
            cfg.levels = cls.default_levels()
        return cfg

    @staticmethod
    def _int(value, default, low, high):
        try:
            v = int(value)
        except (TypeError, ValueError):
            return default
        if v < low or v > high:
            return default
        return v

    @staticmethod
    def default_levels():
        """The 10 default levels from the subject."""
        table = [
            ("Level 1", 10, 10, 5, 42, 90),
            ("Level 2", 15, 15, 8, 0, 180),
            ("Level 3", 20, 21, 13, 0, 200),
            ("Level 4", 25, 25, 16, 0, 250),
            ("Level 5", 30, 30, 19, 0, 300),
            ("Level 6", 35, 35, 23, 0, 350),
            ("Level 7", 39, 39, 26, 0, 400),
            ("Level 8", 42, 42, 30, 0, 450),
            ("Level 9", 45, 45, 33, 0, 500),
            ("Level 10", 20, 20, 8, 0, 550),
        ]
        return [
            LevelConfig(name=name, width=w, height=h, pacgums=p, seed=s, time=t)
            for name, w, h, p, s, t in table
        ]


def _strip_comments(text):
    """Remove full-line ``#`` comments from a JSON-ish text."""
    lines = []
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith("#"):
            continue
        lines.append(line)
    return "\n".join(lines)


def load_config(path):
    """Load and validate a config file, returning a ``GameConfig``.

    Falls back to ``GameConfig`` defaults if the file is missing, invalid
    JSON, or has invalid values.
    """
    try:
        with open(path, "r", encoding="utf-8") as fh:
            text = fh.read()
        data = json.loads(_strip_comments(text))
    except (OSError, ValueError):
        return GameConfig()
    return GameConfig.from_dict(data)


def resolve_highscore_path(filename, base_dir=None):
    """Resolve the highscore file path relative to the project root."""
    if os.path.isabs(filename):
        return filename
    base = base_dir or os.getcwd()
    return os.path.join(base, filename)
