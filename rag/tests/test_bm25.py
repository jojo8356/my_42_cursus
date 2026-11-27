"""Tests for the BM25 lexical retrieval engine."""
import pytest

from models import Chunk, stable_chunk_id
from bm25 import BM25Index, tokenize


def _chunk(text, file_path="f.py", file_type="python", symbols=None):
    text = text
    return Chunk(
        file_path=file_path, text=text,
        first_character_index=0, last_character_index=len(text),
        file_type=file_type, symbols=symbols or [],
        chunk_id=stable_chunk_id(file_path, 0, len(text), text),
    )


def test_tokenize_lowercase_and_stopwords():
    tokens = tokenize("The Scheduler Config is here")
    assert "scheduler" in tokens
    assert "config" in tokens
    assert "the" not in tokens
    assert "is" not in tokens


def test_bm25_ranks_relevant_first():
    idx = BM25Index()
    idx.add(_chunk("The scheduler allocates slots for requests."))
    idx.add(_chunk("The scheduler config controls max size."))
    idx.add(_chunk("Authentication uses a secret key."))
    results = idx.search("scheduler", k=3)
    # Only the two scheduler chunks match (score > 0); the authentication
    # chunk is excluded. Both scheduler chunks are returned.
    assert len(results) == 2
    assert all("scheduler" in r.chunk.text.lower() for r in results)
    assert not any("authentication" in r.chunk.text.lower() for r in results)


def test_bm25_score_positive_for_match():
    idx = BM25Index()
    idx.add(_chunk("vLLM schedules requests using a scheduler"))
    results = idx.search("scheduler", k=1)
    assert results and results[0].score > 0


def test_bm25_no_match_returns_empty():
    idx = BM25Index()
    idx.add(_chunk("authentication key secret"))
    results = idx.search("quantum entanglement", k=3)
    assert results == []


def test_bm25_k_limit():
    idx = BM25Index()
    for i in range(10):
        idx.add(_chunk(f"document number {i} about scheduler"))
    results = idx.search("scheduler", k=3)
    assert len(results) == 3
    assert [r.rank for r in results] == [1, 2, 3]


def test_bm25_symbol_bonus():
    idx = BM25Index(symbol_bonus=10.0)
    idx.add(_chunk("def allocate_slots(): pass", symbols=["allocate_slots"]))
    idx.add(_chunk("scheduler text without symbol", symbols=[]))
    results = idx.search("allocate_slots", k=2)
    # The chunk whose symbol matches the query should rank first.
    assert results[0].chunk.symbols == ["allocate_slots"]


def test_bm25_docs_bonus():
    idx = BM25Index(docs_bonus=5.0)
    idx.add(_chunk("scheduler explanation in docs", file_type="markdown"))
    idx.add(_chunk("scheduler implementation in code", file_type="python"))
    results = idx.search("scheduler", k=2)
    # Documentation chunk gets a bonus -> ranks first on a tie of content.
    assert results[0].chunk.file_type == "markdown"


def test_bm25_query_expansion():
    idx = BM25Index(expansions={"kv": ["key value cache"]})
    idx.add(_chunk("key value cache stores tokens"))
    idx.add(_chunk("unrelated content about trees"))
    results = idx.search("kv cache", k=2)
    assert results[0].chunk.text.startswith("key value cache")


def test_bm25_save_load(tmp_path):
    idx = BM25Index()
    idx.add(_chunk("scheduler allocates slots"))
    idx.add(_chunk("authentication uses a key"))
    path = str(tmp_path / "bm25.json")
    idx.save(path)
    idx2 = BM25Index.load(path)
    assert len(idx2.chunks) == 2
    results = idx2.search("scheduler", k=1)
    assert results and results[0].score > 0


def test_bm25_file_type_filter():
    idx = BM25Index()
    idx.add(_chunk("scheduler docs", file_type="markdown"))
    idx.add(_chunk("scheduler code", file_type="python"))
    results = idx.search("scheduler", k=2, file_type_filter="python")
    assert len(results) == 1
    assert results[0].chunk.file_type == "python"
