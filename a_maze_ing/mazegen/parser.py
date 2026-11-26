"""Strict KEY=VALUE configuration file parser."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .errors import ConfigError

MANDATORY = ("WIDTH", "HEIGHT", "ENTRY", "EXIT", "OUTPUT_FILE", "PERFECT")
KNOWN = set(MANDATORY) | {"SEED", "ALGORITHM"}


@dataclass
class Config:
    """A validated configuration."""

    width: int
    height: int
    entry: tuple[int, int]
    exit: tuple[int, int]
    output_file: str
    perfect: bool
    seed: int | None = None
    algorithm: str = "dfs"


def _parse_int(key: str, value: str) -> int:
    """Parse `value` as an integer, raising ConfigError otherwise."""
    try:
        return int(value)
    except (TypeError, ValueError) as exc:
        raise ConfigError(f"{key} must be an integer, got {value!r}") from exc


def _parse_coords(key: str, value: str) -> tuple[int, int]:
    """Parse a 'x,y' coordinate pair, raising ConfigError otherwise."""
    parts = value.split(",")
    if len(parts) != 2:
        raise ConfigError(f"{key} must be two numbers 'x,y', got {value!r}")
    try:
        x, y = int(parts[0]), int(parts[1])
    except (TypeError, ValueError) as exc:
        raise ConfigError(f"{key} must be two integers 'x,y', got {value!r}") from exc
    return x, y


def _parse_bool(key: str, value: str) -> bool:
    """Parse a True/False value, raising ConfigError otherwise."""
    low = value.strip().lower()
    if low == "true":
        return True
    if low == "false":
        return False
    raise ConfigError(f"{key} must be True or False, got {value!r}")


def parse_text(text: str) -> Config:
    """Parse a configuration from its text content."""
    values: dict[str, str] = {}
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            raise ConfigError(f"each line must be KEY=VALUE, got {raw!r}")
        key, value = line.split("=", 1)
        key = key.strip().upper()
        value = value.strip()
        if not key:
            raise ConfigError(f"a line has an empty key: {raw!r}")
        if key not in KNOWN:
            raise ConfigError(f"unknown configuration key: {key}")
        if key in values:
            raise ConfigError(f"{key} is given twice in the configuration")
        values[key] = value

    missing = [k for k in MANDATORY if k not in values]
    if missing:
        raise ConfigError("missing mandatory key(s): " + ", ".join(missing))

    width = _parse_int("WIDTH", values["WIDTH"])
    height = _parse_int("HEIGHT", values["HEIGHT"])
    entry = _parse_coords("ENTRY", values["ENTRY"])
    exit = _parse_coords("EXIT", values["EXIT"])
    output_file = values["OUTPUT_FILE"]
    perfect = _parse_bool("PERFECT", values["PERFECT"])

    if width <= 0:
        raise ConfigError(f"WIDTH must be positive, got {width}")
    if height <= 0:
        raise ConfigError(f"HEIGHT must be positive, got {height}")
    if not output_file:
        raise ConfigError("OUTPUT_FILE must not be empty")

    for name, (x, y) in (("ENTRY", entry), ("EXIT", exit)):
        if not (0 <= x < width and 0 <= y < height):
            raise ConfigError(
                f"{name} ({x},{y}) is outside the maze bounds "
                f"({width}x{height})"
            )
    if entry == exit:
        raise ConfigError("ENTRY and EXIT must be different cells")

    seed = None
    if "SEED" in values and values["SEED"] != "":
        seed = _parse_int("SEED", values["SEED"])

    algorithm = values.get("ALGORITHM", "dfs").lower()
    if algorithm not in ("dfs", "prim"):
        raise ConfigError(f"ALGORITHM must be 'dfs' or 'prim', got {algorithm!r}")

    return Config(width, height, entry, exit, output_file, perfect, seed, algorithm)


def parse_file(filename: str | Path) -> Config:
    """Read and parse a configuration file."""
    path = Path(filename)
    try:
        data = path.read_bytes()
    except FileNotFoundError as exc:
        raise ConfigError(f"configuration file not found: {path}") from exc
    except IsADirectoryError as exc:
        raise ConfigError(f"configuration path is a directory: {path}") from exc
    except OSError as exc:
        raise ConfigError(f"cannot read the configuration file {path}: {exc}") from exc
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ConfigError(f"the configuration file is not valid UTF-8: {path}") from exc
    return parse_text(text)
