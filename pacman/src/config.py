"""
Configuration parsing for Pacman.

The game reads a JSON config file with game-wide settings and a ``levels``
array. Invalid or ambiguous values use safe defaults; in particular, booleans,
floats and numeric strings are not accepted as integers. Full-line comments
beginning with ``#`` are ignored.
"""
from __future__ import annotations

import json
import os
from collections.abc import Mapping
from dataclasses import dataclass, field
from os import PathLike


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
    def from_dict(cls, data: object) -> LevelConfig:
        if not isinstance(data, Mapping):
            return cls()
        return cls(
            name=cls._string(data.get("name"), "level"),
            width=cls._int(data.get("width"), 10, 1, 100),
            height=cls._int(data.get("height"), 10, 1, 100),
            pacgums=cls._int(data.get("pacgums"), 5, 0, 10000),
            seed=cls._int(data.get("seed"), 0, -(10**9), 10**9),
            time=cls._int(data.get("time"), 90, 1, 100000),
        )

    @staticmethod
    def _int(value: object, default: int, low: int, high: int) -> int:
        if type(value) is not int or value < low or value > high:
            return default
        return value

    @staticmethod
    def _string(value: object, default: str) -> str:
        if not isinstance(value, str) or not value.strip():
            return default
        return value.strip()


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
    levels: list[LevelConfig] = field(
        default_factory=lambda: GameConfig.default_levels()
    )

    @classmethod
    def from_dict(cls, data: object) -> GameConfig:
        if not isinstance(data, Mapping):
            return cls()
        config = cls(
            highscore_filename=cls._string(
                data.get("highscore_filename"), "highscores.json"
            ),
            lives=cls._int(data.get("lives"), 3, 1, 99),
            screen_width=cls._int(data.get("screen_width"), 1000, 100, 10000),
            screen_height=cls._int(
                data.get("screen_height"), 1000, 100, 10000
            ),
            level_max_time=cls._int(data.get("level_max_time"), 90, 1, 100000),
            points_per_pacgum=cls._int(
                data.get("points_per_pacgum"), 10, 0, 100000
            ),
            points_per_super_pacgum=cls._int(
                data.get("points_per_super_pacgum"), 50, 0, 100000
            ),
            points_per_ghost=cls._int(
                data.get("points_per_ghost"), 200, 0, 100000
            ),
        )
        raw_levels = data.get("levels")
        if (
            isinstance(raw_levels, list)
            and raw_levels
            and all(isinstance(item, Mapping) for item in raw_levels)
        ):
            config.levels = [
                LevelConfig.from_dict(item) for item in raw_levels
            ]
        else:
            config.levels = cls.default_levels()
        return config

    @staticmethod
    def _int(value: object, default: int, low: int, high: int) -> int:
        if type(value) is not int or value < low or value > high:
            return default
        return value

    @staticmethod
    def _string(value: object, default: str) -> str:
        if not isinstance(value, str) or not value.strip():
            return default
        return value.strip()

    @staticmethod
    def default_levels() -> list[LevelConfig]:
        """Return the ten default levels from the subject."""
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
            LevelConfig(
                name=name,
                width=width,
                height=height,
                pacgums=pacgums,
                seed=seed,
                time=level_time,
            )
            for name, width, height, pacgums, seed, level_time in table
        ]


def _strip_comments(text: str) -> str:
    """Remove full-line ``#`` comments from JSON-like text."""
    lines = []
    for line in text.splitlines():
        if line.strip().startswith("#"):
            continue
        lines.append(line)
    return "\n".join(lines)


def load_config(path: str | PathLike[str]) -> GameConfig:
    """Load and validate a config file, or return defaults on invalid input."""
    try:
        with open(path, "r", encoding="utf-8") as config_file:
            text = config_file.read()
        data = json.loads(_strip_comments(text))
    except (OSError, ValueError):
        return GameConfig()
    return GameConfig.from_dict(data)


def resolve_highscore_path(
    filename: str,
    base_dir: str | PathLike[str] | None = None,
) -> str:
    """Resolve the high-score file path relative to the project root."""
    if os.path.isabs(filename):
        return filename
    base = base_dir or os.getcwd()
    return os.path.join(base, filename)
