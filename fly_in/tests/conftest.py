"""Shared pytest fixtures for the Fly-In test suite."""
from __future__ import annotations

from pathlib import Path

import pytest

from src.parser import MapParser

MAP_ROOT = Path(__file__).resolve().parent.parent / "maps"


@pytest.fixture
def map_root() -> Path:
    """Return the absolute path of the supplied maps directory."""
    return MAP_ROOT


@pytest.fixture
def parser() -> MapParser:
    """Return a fresh map parser."""
    return MapParser()


@pytest.fixture
def map_path(map_root: Path) -> Path:
    """Return the easy linear-path map, the canonical smoke-test map."""
    return map_root / "easy" / "01_linear_path.txt"
