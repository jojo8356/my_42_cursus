"""Tests for the high-score store."""
import json

import pytest

from highscore import validate_name, validate_score, HighscoreStore


def test_validate_name_valid():
    assert validate_name("Alice") == "Alice"
    assert validate_name("Player 1") == "Player 1"
    assert validate_name("A1B2C3D4E5") == "A1B2C3D4E5"


def test_validate_name_invalid():
    with pytest.raises(ValueError):
        validate_name("")  # empty is technically valid (0 chars) but stripped
    with pytest.raises(ValueError):
        validate_name("ThisNameIsTooLong")
    with pytest.raises(ValueError):
        validate_name("Has!Special@Chars")
    with pytest.raises(ValueError):
        validate_name(123)  # not a string


def test_validate_name_empty_invalid():
    # Empty / whitespace-only names are rejected.
    with pytest.raises(ValueError):
        validate_name("   ")
    with pytest.raises(ValueError):
        validate_name("")


def test_validate_score_valid():
    assert validate_score(0) == 0
    assert validate_score(1000) == 1000
    assert validate_score("500") == 500


def test_validate_score_invalid():
    with pytest.raises(ValueError):
        validate_score(-1)
    with pytest.raises(ValueError):
        validate_score("abc")
    with pytest.raises(ValueError):
        validate_score(None)


def test_add_and_top(tmp_path):
    store = HighscoreStore(str(tmp_path / "hs.json"))
    store.add("Alice", 100)
    store.add("Bob", 300)
    store.add("Cara", 200)
    top = store.top()
    assert top[0] == {"name": "Bob", "score": 300}
    assert top[1] == {"name": "Cara", "score": 200}
    assert top[2] == {"name": "Alice", "score": 100}


def test_top_ten_limit(tmp_path):
    store = HighscoreStore(str(tmp_path / "hs.json"))
    for i in range(15):
        store.add(f"P{i}", i * 10)
    top = store.top()
    assert len(top) == 10
    # Highest scores retained.
    assert top[0]["score"] == 140
    assert top[-1]["score"] == 50


def test_save_and_load(tmp_path):
    path = tmp_path / "hs.json"
    store = HighscoreStore(str(path))
    store.add("Alice", 100)
    store.add("Bob", 300)
    store.save()
    assert path.exists()

    store2 = HighscoreStore(str(path))
    store2.load()
    assert store2.top() == [{"name": "Bob", "score": 300},
                            {"name": "Alice", "score": 100}]


def test_load_invalid_file(tmp_path):
    path = tmp_path / "hs.json"
    path.write_text("not json")
    store = HighscoreStore(str(path))
    store.load()
    assert store.top() == []


def test_is_new_record(tmp_path):
    store = HighscoreStore(str(tmp_path / "hs.json"))
    for i in range(15):
        store.add(f"P{i}", i * 10)
    # After keeping the top 10, the lowest kept score is 50.
    # A score below 50 is not a new record; strictly above 50 is.
    assert store.is_new_record(5) is False
    assert store.is_new_record(50) is False
    assert store.is_new_record(51) is True
    assert store.is_new_record(100) is True


def test_add_invalid_raises(tmp_path):
    store = HighscoreStore(str(tmp_path / "hs.json"))
    with pytest.raises(ValueError):
        store.add("Invalid Name!", 100)
    with pytest.raises(ValueError):
        store.add("Alice", -5)
