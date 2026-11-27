"""
Reciprocal Rank Fusion (RRF) for hybrid retrieval.

Instead of comparing raw scores (BM25 vs cosine have completely different
scales), RRF combines rankings: each engine contributes ``1 / (k + rank)``
for every result it returned, and the contributions are summed. Chunks
consistently ranked well by both engines rise to the top.

Deterministic and robust: the final order depends only on ranks, not scores.
"""
from collections import defaultdict

from models import SearchResult

DEFAULT_RRF_K = 60


def rrf_fuse(ranked_lists, k=DEFAULT_RRF_K, top_n=None):
    """Fuse several ranked lists into a single ranked list of ``SearchResult``.

    ``ranked_lists`` is a list of lists of ``SearchResult`` (one per engine).
    Returns a new list of ``SearchResult`` sorted by fused score. Ties are
    broken deterministically by (score desc, then the earliest appearance).
    """
    fused = defaultdict(float)
    order = {}  # chunk_id -> first appearance index (for stable tie-break)
    metadata = {}  # chunk_id -> the SearchResult
    appearance = 0

    for ranked in ranked_lists:
        for rank, result in enumerate(ranked, start=1):
            cid = result.chunk.chunk_id or (
                result.chunk.file_path,
                result.chunk.first_character_index)
            fused[cid] += 1.0 / (k + rank)
            if cid not in metadata:
                metadata[cid] = result
                order[cid] = appearance
                appearance += 1

    ranked = sorted(
        fused.items(), key=lambda item: (-item[1], order[item[0]]))

    results = []
    for i, (cid, score) in enumerate(ranked):
        original = metadata[cid]
        results.append(SearchResult(
            chunk=original.chunk, score=score, rank=i + 1))
    if top_n is not None:
        results = results[:top_n]
    return results
