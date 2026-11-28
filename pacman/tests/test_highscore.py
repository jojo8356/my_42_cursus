"""Tests for high-score validation, ordering and JSON persistence."""
import json
from pathlib import Path

import pytest

from highscore import HighscoreStore, validate_name, validate_score


def test_validate_name_valid() -> None:
    assert validate_name("Alice") == "Alice"
    assert validate_name("Player 1") == "Player 1"
    assert validate_name("A1B2C3D4E5") == "A1B2C3D4E5"


@pytest.mark.parametrize(
    "name",
    ["", "   ", "ThisNameIsTooLong", "Has!Special@Chars", 123],
)
def test_validate_name_invalid(name: object) -> None:
    with pytest.raises(ValueError):
        validate_name(name)


def test_validate_score_valid() -> None:
    assert validate_score(0) == 0
    assert validate_score(1000) == 1000


@pytest.mark.parametrize("score", [-1, "abc", None, True, 3.5, "500"])
def test_validate_score_rejects_invalid_or_ambiguous_values(
    score: object,
) -> None:
    with pytest.raises(ValueError):
        validate_score(score)


def test_add_and_top(tmp_path: Path) -> None:
    store = HighscoreStore(tmp_path / "hs.json")
    store.add("Alice", 100)
    store.add("Bob", 300)
    store.add("Cara", 200)

    assert store.top() == [
        {"name": "Bob", "score": 300},
        {"name": "Cara", "score": 200},
        {"name": "Alice", "score": 100},
    ]


def test_top_ten_limit(tmp_path: Path) -> None:
    store = HighscoreStore(tmp_path / "hs.json")
    for index in range(15):
        store.add(f"P{index}", index * 10)

    top = store.top()
    assert len(top) == 10
    assert top[0]["score"] == 140
    assert top[-1]["score"] == 50


def test_save_and_load(tmp_path: Path) -> None:
    path = tmp_path / "hs.json"
    store = HighscoreStore(path)
    store.add("Alice", 100)
    store.add("Bob", 300)
    store.save()
    assert path.exists()

    reloaded = HighscoreStore(path)
    reloaded.load()
    assert reloaded.top() == [
        {"name": "Bob", "score": 300},
        {"name": "Alice", "score": 100},
    ]


def test_load_invalid_file(tmp_path: Path) -> None:
    path = tmp_path / "hs.json"
    path.write_text("not json", encoding="utf-8")
    store = HighscoreStore(path)
    store.load()
    assert store.top() == []


def test_load_ignores_invalid_record_shapes(tmp_path: Path) -> None:
    path = tmp_path / "hs.json"
    path.write_text(
        json.dumps(
            [
                None,
                {},
                {"score": 10},
                {"name": "Valid", "score": 10},
            ]
        ),
        encoding="utf-8",
    )
    store = HighscoreStore(path)

    assert store.load() == [{"name": "Valid", "score": 10}]
    path.write_text(json.dumps({"name": "not a list"}), encoding="utf-8")
    assert store.load() == []


def test_load_discards_ambiguous_scores_and_limits_records(
    tmp_path: Path,
) -> None:
    path = tmp_path / "hs.json"
    records = [
        {"name": "Valid", "score": 100},
        {"name": "Boolean", "score": True},
        {"name": "Float", "score": 25.0},
        {"name": "String", "score": "75"},
        *[{"name": f"P{index}", "score": index} for index in range(20)],
    ]
    path.write_text(json.dumps(records), encoding="utf-8")

    store = HighscoreStore(path)
    store.load()

    assert len(store.top()) == store.TOP_N
    assert store.top()[0] == {"name": "Valid", "score": 100}
    assert all(record["name"] not in {"Boolean", "Float", "String"}
               for record in store.top())


def test_is_new_record(tmp_path: Path) -> None:
    store = HighscoreStore(tmp_path / "hs.json")
    for index in range(15):
        store.add(f"P{index}", index * 10)

    assert store.is_new_record(5) is False
    assert store.is_new_record(50) is False
    assert store.is_new_record(51) is True
    assert store.is_new_record(100) is True
    with pytest.raises(ValueError):
        store.is_new_record(True)


def test_add_invalid_raises(tmp_path: Path) -> None:
    store = HighscoreStore(tmp_path / "hs.json")
    with pytest.raises(ValueError):
        store.add("Invalid Name!", 100)
    with pytest.raises(ValueError):
        store.add("Alice", -5)
    with pytest.raises(ValueError):
        store.add("Alice", 8.5)
