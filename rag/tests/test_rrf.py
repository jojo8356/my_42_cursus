"""Tests for Reciprocal Rank Fusion."""
from models import SearchResult, Chunk, stable_chunk_id
from rrf import rrf_fuse


def _res(text, file_path="f", rank=1, score=1.0):
    chunk = Chunk(file_path, text, 0, len(text), "python",
                  chunk_id=stable_chunk_id(file_path, 0, len(text), text))
    return SearchResult(chunk=chunk, score=score, rank=rank)


def test_rrf_fuses_rankings():
    list_a = [_res("alpha", "a", rank=1), _res("beta", "b", rank=2)]
    list_b = [_res("beta", "b", rank=1), _res("alpha", "a", rank=2)]
    fused = rrf_fuse([list_a, list_b])
    # alpha and beta appear in both lists; both should be present.
    texts = [r.chunk.text for r in fused]
    assert "alpha" in texts
    assert "beta" in texts
    # Both get equal fused score (symmetric), so order is stable by
    # appearance; both ranks are 1 and 2.
    assert fused[0].rank == 1
    assert fused[1].rank == 2


def test_rrf_chunk_in_both_ranks_higher():
    # A chunk ranked #1 by both engines should outscore a chunk ranked #2
    # by only one engine.
    list_a = [_res("common", "c", rank=1), _res("only_a", "oa", rank=2)]
    list_b = [_res("common", "c", rank=1)]
    fused = rrf_fuse([list_a, list_b])
    assert fused[0].chunk.text == "common"


def test_rrf_top_n():
    list_a = [_res(f"doc{i}", f"d{i}", rank=i + 1) for i in range(10)]
    fused = rrf_fuse([list_a], top_n=3)
    assert len(fused) == 3


def test_rrf_deterministic():
    list_a = [_res("x", "a", rank=1), _res("y", "b", rank=2)]
    list_b = [_res("y", "b", rank=1), _res("x", "a", rank=2)]
    f1 = rrf_fuse([list_a, list_b])
    f2 = rrf_fuse([list_a, list_b])
    assert [r.chunk.text for r in f1] == [r.chunk.text for r in f2]
