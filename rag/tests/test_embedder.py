"""Tests for embeddings and the vector store."""
import pytest

from embedder import HashingEmbedder, make_embedder
from vector_store import VectorStore, cosine
from models import Chunk, stable_chunk_id


def test_hashing_embedder_deterministic():
    emb = HashingEmbedder(dimensions=64)
    v1 = emb.embed_text("the scheduler allocates slots")
    v2 = emb.embed_text("the scheduler allocates slots")
    assert v1 == v2
    assert len(v1) == 64


def test_hashing_embedder_normalized():
    emb = HashingEmbedder(dimensions=64)
    v = emb.embed_text("some text with several tokens")
    norm = sum(x * x for x in v) ** 0.5
    assert abs(norm - 1.0) < 1e-9


def test_hashing_embedder_different_text_different_vector():
    emb = HashingEmbedder(dimensions=64)
    a = emb.embed_text("scheduler")
    b = emb.embed_text("authentication")
    assert a != b


def test_embed_documents():
    emb = HashingEmbedder(dimensions=32)
    vecs = emb.embed_documents(["alpha", "beta", "gamma"])
    assert len(vecs) == 3
    for v in vecs:
        assert len(v) == 32


def test_make_embedder_factory():
    assert isinstance(make_embedder("hashing"), HashingEmbedder)
    with pytest.raises(ValueError):
        make_embedder("unknown")


def test_cosine_similarity():
    assert cosine([1, 0], [1, 0]) == pytest.approx(1.0)
    assert cosine([1, 0], [0, 1]) == pytest.approx(0.0)
    assert cosine([1, 0], [-1, 0]) == pytest.approx(-1.0)
    assert cosine([], [1]) == 0.0


def test_vector_store_search():
    store = VectorStore()
    emb = HashingEmbedder(dimensions=64)
    c1 = Chunk("a.py", "scheduler", 0, 8, "python",
               chunk_id=stable_chunk_id("a.py", 0, 8, "scheduler"))
    c2 = Chunk("b.py", "authentication key", 0, 17, "python",
               chunk_id=stable_chunk_id("b.py", 0, 17, "authentication key"))
    store.add(c1, emb.embed_text("scheduler"))
    store.add(c2, emb.embed_text("authentication key"))
    results = store.search(emb.embed_text("scheduler"), k=1)
    assert results[0].chunk.file_path == "a.py"


def test_vector_store_save_load(tmp_path):
    store = VectorStore()
    emb = HashingEmbedder(dimensions=64)
    c = Chunk("a.py", "scheduler", 0, 8, "python",
              chunk_id=stable_chunk_id("a.py", 0, 8, "scheduler"))
    store.add(c, emb.embed_text("scheduler"))
    path = str(tmp_path / "vector.json")
    store.save(path)
    store2 = VectorStore.load(path)
    assert len(store2) == 1
    results = store2.search(emb.embed_text("scheduler"), k=1)
    assert results and results[0].chunk.file_path == "a.py"
