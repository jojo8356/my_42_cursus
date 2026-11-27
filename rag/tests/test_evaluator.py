"""Tests for Recall@k evaluation."""
from models import Chunk, SearchResult, stable_chunk_id
from evaluator import evaluate


def _result(file_path, text="", score=1.0):
    chunk = Chunk(file_path, text, 0, len(text), "python",
                  chunk_id=stable_chunk_id(file_path, 0, len(text), text))
    return SearchResult(chunk=chunk, score=score, rank=1)


def test_recall_perfect():
    results = {
        "q1": [_result("a.py"), _result("b.py")],
        "q2": [_result("c.py")],
    }
    dataset = [
        {"question": "q1", "expected": ["a.py"]},
        {"question": "q2", "expected": ["c.py"]},
    ]
    report = evaluate(results, dataset)
    assert report["total"] == 2
    assert report["recall_at_1"] == 1.0
    assert report["recall_at_3"] == 1.0
    assert report["recall_at_5"] == 1.0


def test_recall_partial():
    results = {
        "q1": [_result("wrong.py"), _result("a.py")],  # expected at rank 2
        "q2": [_result("wrong.py")],  # expected missing entirely
    }
    dataset = [
        {"question": "q1", "expected": ["a.py"]},
        {"question": "q2", "expected": ["c.py"]},
    ]
    report = evaluate(results, dataset)
    # q1's expected doc is at rank 2 (not in top-1); q2's is missing.
    assert report["recall_at_1"] == 0.0
    assert report["recall_at_3"] == 0.5
    assert report["recall_at_5"] == 0.5


def test_recall_empty_dataset():
    report = evaluate({}, [])
    assert report["total"] == 0
    assert report["recall_at_1"] == 0.0


def test_recall_with_char_range():
    results = {
        "q1": [_result("a.py")],
    }
    dataset = [
        {"question": "q1",
         "expected": [{"file_path": "a.py",
                       "first_character_index": 0,
                       "last_character_index": 100}]},
    ]
    report = evaluate(results, dataset)
    assert report["recall_at_1"] == 1.0


def test_recall_range_mismatch():
    results = {
        "q1": [_result("a.py")],  # wrong offsets
    }
    dataset = [
        {"question": "q1",
         "expected": [{"file_path": "a.py",
                       "first_character_index": 50,
                       "last_character_index": 100}]},
    ]
    report = evaluate(results, dataset)
    assert report["recall_at_1"] == 0.0
