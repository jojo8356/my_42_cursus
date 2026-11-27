"""Tests for the unified retriever (BM25, semantic, hybrid, fallback)."""
import pytest

from models import Chunk, stable_chunk_id
from bm25 import BM25Index
from embedder import HashingEmbedder
from vector_store import VectorStore
from retriever import Retriever


def _chunk(text, file_path="f.py", file_type="python"):
    return Chunk(file_path, text, 0, len(text), file_type,
                 chunk_id=stable_chunk_id(file_path, 0, len(text), text))


@pytest.fixture
def corpus():
    chunks = [
        _chunk("The scheduler allocates slots for incoming requests."),
        _chunk("The scheduler config controls the maximum size."),
        _chunk("Authentication uses a secret key to validate."),
        _chunk("The cache stores token embeddings for reuse."),
    ]
    return chunks


@pytest.fixture
def retriever(corpus):
    bm25 = BM25Index()
    bm25.add_all(corpus)
    embedder = HashingEmbedder(dimensions=64)
    store = VectorStore()
    for c in corpus:
        store.add(c, embedder.embed_text(c.text))
    return Retriever(bm25=bm25, vector_store=store, embedder=embedder)


def test_search_bm25(retriever):
    results = retriever.search("scheduler", k=2, mode="bm25")
    assert len(results) == 2
    assert "scheduler" in results[0].chunk.text.lower()


def test_search_semantic(retriever):
    results = retriever.search("scheduler", k=2, mode="semantic")
    assert len(results) == 2
    # Semantic retrieval also surfaces scheduler-related chunks.
    assert results[0].score > 0


def test_search_hybrid(retriever):
    results = retriever.search("scheduler", k=3, mode="hybrid")
    assert 1 <= len(results) <= 3
    assert all(r.rank >= 1 for r in results)


def test_hybrid_fallback_without_vector_store(corpus):
    bm25 = BM25Index()
    bm25.add_all(corpus)
    retriever = Retriever(bm25=bm25, vector_store=None,
                          embedder=HashingEmbedder(dimensions=64))
    warnings = []
    results = retriever.search_hybrid("scheduler", k=2,
                                      warn=warnings.append)
    # Falls back to BM25 and emits a warning.
    assert results
    assert warnings
    assert "falling back" in warnings[0].lower()


def test_candidate_selection(corpus):
    bm25 = BM25Index()
    bm25.add_all(corpus)
    retriever = Retriever(bm25=bm25,
                          vector_store=VectorStore(),
                          embedder=HashingEmbedder(dimensions=64))
    # Candidate count is capped by corpus size.
    assert retriever._candidate_count(2, len(corpus)) <= len(corpus)
    # At least the min candidates when corpus is large enough.
    assert retriever._candidate_count(2, 1000) >= retriever.min_candidates


def test_unknown_mode_raises(retriever):
    with pytest.raises(ValueError):
        retriever.search("x", mode="nonexistent")


def test_semantic_unavailable_returns_empty():
    retriever = Retriever(bm25=BM25Index(), vector_store=VectorStore(),
                          embedder=HashingEmbedder(dimensions=64))
    assert retriever.search_semantic("x", k=3) == []
