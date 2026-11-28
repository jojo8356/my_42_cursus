"""Tests for the configuration parser and strict value validation."""
import json
import os
from pathlib import Path

import pytest

from config import GameConfig, LevelConfig, load_config


def test_non_object_config_values_use_defaults() -> None:
    assert GameConfig.from_dict(None).lives == 3
    assert LevelConfig.from_dict(None) == LevelConfig()


def test_default_levels_count() -> None:
    config = GameConfig()
    assert len(config.levels) == 10
    first = config.levels[0]
    assert first.width == 10
    assert first.height == 10
    assert first.pacgums == 5
    assert first.seed == 42
    assert first.time == 90


def test_load_valid_config(tmp_path: Path) -> None:
    path = tmp_path / "config.json"
    data = {
        "lives": 5,
        "points_per_pacgum": 20,
        "levels": [
            {
                "name": "Custom",
                "width": 8,
                "height": 8,
                "pacgums": 3,
                "seed": 7,
                "time": 120,
            },
        ],
    }
    path.write_text(json.dumps(data), encoding="utf-8")
    config = load_config(path)
    assert config.lives == 5
    assert config.points_per_pacgum == 20
    assert len(config.levels) == 1
    assert config.levels[0].name == "Custom"
    assert config.levels[0].width == 8
    assert config.levels[0].seed == 7


def test_load_config_ignores_comments(tmp_path: Path) -> None:
    path = tmp_path / "config.json"
    content = """
    # This is a comment
    {
      # another comment
      "lives": 4,
      "levels": []
    }
    """
    path.write_text(content, encoding="utf-8")
    config = load_config(path)
    assert config.lives == 4


def test_load_invalid_json_falls_back(tmp_path: Path) -> None:
    path = tmp_path / "bad.json"
    path.write_text("{ not valid json ", encoding="utf-8")
    config = load_config(path)
    assert config.lives == 3
    assert len(config.levels) == 10


def test_load_missing_file_falls_back(tmp_path: Path) -> None:
    config = load_config(tmp_path / "nope.json")
    assert config.lives == 3
    assert len(config.levels) == 10


def test_invalid_values_fall_back_to_defaults() -> None:
    config = GameConfig.from_dict(
        {
            "lives": -5,
            "points_per_pacgum": "abc",
            "screen_width": 999999,
        }
    )
    assert config.lives == 3
    assert config.points_per_pacgum == 10
    assert config.screen_width == 1000


def test_level_invalid_values_fall_back() -> None:
    config = GameConfig.from_dict(
        {
            "levels": [
                {
                    "width": -1,
                    "height": 0,
                    "pacgums": "x",
                    "seed": 10**12,
                }
            ],
        }
    )
    level = config.levels[0]
    assert level.width == 10
    assert level.height == 10
    assert level.pacgums == 5
    assert level.seed == 0


@pytest.mark.parametrize("value", [True, 5.0, "5"])
def test_ambiguous_numeric_values_are_not_coerced(value: object) -> None:
    config = GameConfig.from_dict({"lives": value})
    level = LevelConfig.from_dict({"width": value, "seed": value})

    assert config.lives == 3
    assert level.width == 10
    assert level.seed == 0


def test_mixed_invalid_level_list_is_rejected() -> None:
    config = GameConfig.from_dict(
        {"levels": [{"name": "valid-looking"}, "not a level"]}
    )

    assert len(config.levels) == 10
    assert config.levels[0].name == "Level 1"


def test_invalid_text_settings_use_safe_defaults() -> None:
    config = GameConfig.from_dict(
        {
            "highscore_filename": 123,
            "levels": [{"name": 42}],
        }
    )

    assert config.highscore_filename == "highscores.json"
    assert config.levels[0].name == "level"


def test_valid_text_settings_are_trimmed() -> None:
    config = GameConfig.from_dict(
        {
            "highscore_filename": " scores.json ",
            "levels": [{"name": " Custom "}],
        }
    )

    assert config.highscore_filename == "scores.json"
    assert config.levels[0].name == "Custom"


def test_resolve_highscore_path(tmp_path: Path) -> None:
    from config import resolve_highscore_path

    path = resolve_highscore_path("highscores.json", tmp_path)
    assert path == os.path.join(str(tmp_path), "highscores.json")
    absolute_path = resolve_highscore_path("/tmp/x.json", tmp_path)
    assert absolute_path == "/tmp/x.json"
