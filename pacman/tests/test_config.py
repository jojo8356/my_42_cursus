"""Tests for the configuration parser."""
import json
import os

import pytest

from config import GameConfig, LevelConfig, load_config


def test_default_levels_count():
    cfg = GameConfig()
    assert len(cfg.levels) == 10
    # First default level matches the subject table.
    first = cfg.levels[0]
    assert first.width == 10
    assert first.height == 10
    assert first.pacgums == 5
    assert first.seed == 42
    assert first.time == 90


def test_load_valid_config(tmp_path):
    path = tmp_path / "config.json"
    data = {
        "lives": 5,
        "points_per_pacgum": 20,
        "levels": [
            {"name": "Custom", "width": 8, "height": 8, "pacgums": 3,
             "seed": 7, "time": 120},
        ],
    }
    path.write_text(json.dumps(data))
    cfg = load_config(str(path))
    assert cfg.lives == 5
    assert cfg.points_per_pacgum == 20
    assert len(cfg.levels) == 1
    assert cfg.levels[0].name == "Custom"
    assert cfg.levels[0].width == 8
    assert cfg.levels[0].seed == 7


def test_load_config_ignores_comments(tmp_path):
    path = tmp_path / "config.json"
    content = """
    # This is a comment
    {
      # another comment
      "lives": 4,
      "levels": []
    }
    """
    path.write_text(content)
    cfg = load_config(str(path))
    assert cfg.lives == 4


def test_load_invalid_json_falls_back(tmp_path):
    path = tmp_path / "bad.json"
    path.write_text("{ not valid json ")
    cfg = load_config(str(path))
    # Falls back to defaults.
    assert cfg.lives == 3
    assert len(cfg.levels) == 10


def test_load_missing_file_falls_back(tmp_path):
    cfg = load_config(str(tmp_path / "nope.json"))
    assert cfg.lives == 3
    assert len(cfg.levels) == 10


def test_invalid_values_fall_back_to_defaults():
    cfg = GameConfig.from_dict({
        "lives": -5,
        "points_per_pacgum": "abc",
        "screen_width": 999999,
    })
    assert cfg.lives == 3
    assert cfg.points_per_pacgum == 10
    assert cfg.screen_width == 1000


def test_level_invalid_values_fall_back():
    cfg = GameConfig.from_dict({
        "levels": [{"width": -1, "height": 0, "pacgums": "x", "seed": 10**12}],
    })
    level = cfg.levels[0]
    assert level.width == 10
    assert level.height == 10
    assert level.pacgums == 5
    assert level.seed == 0


def test_resolve_highscore_path(tmp_path):
    from config import resolve_highscore_path
    p = resolve_highscore_path("highscores.json", tmp_path)
    assert p == os.path.join(str(tmp_path), "highscores.json")
    # Absolute path is returned as-is.
    abs_p = resolve_highscore_path("/tmp/x.json", tmp_path)
    assert abs_p == "/tmp/x.json"
