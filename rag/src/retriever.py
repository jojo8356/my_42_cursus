"""
Retrieval orchestration: BM25, semantic and hybrid search.

The retriever unifies the three retrieval modes behind a single interface and
implements the graceful fallback described in the subject: if the semantic
index is missing or incompatible, hybrid search degrades to BM25 with a
warning rather than aborting.
"""
from bm25 import BM25Index
from embedder import HashingEmbedder
from vector_store import VectorStore
from rrf import rrf_fuse

DEFAULT_CANDIDATE_MULTIPLIER = 3
DEFAULT_MIN_CANDIDATES = 50


class Retriever:
    """Combines a BM25 index and an optional semantic vector store."""

    def __init__(self, bm25=None, vector_store=None, embedder=None,
                 candidate_multiplier=DEFAULT_CANDIDATE_MULTIPLIER,
                 min_candidates=DEFAULT_MIN_CANDIDATES):
        self.bm25 = bm25 or BM25Index()
        self.vector_store = vector_store
        self.embedder = embedder or HashingEmbedder()
        self.candidate_multiplier = candidate_multiplier
        self.min_candidates = min_candidates

    # ------------------------------------------------------------------ helpers
    def _candidate_count(self, k, corpus_size):
        """Number of candidates to pull from each engine before fusion."""
        count = max(k * self.candidate_multiplier, self.min_candidates)
        return min(count, max(corpus_size, 1))

    def _semantic_available(self):
        return self.vector_store is not None and len(self.vector_store) > 0

    # ------------------------------------------------------------------ modes
    def search_bm25(self, query, k=5, file_type_filter=None):
        return self.bm25.search(query, k=k, file_type_filter=file_type_filter)

    def search_semantic(self, query, k=5):
        if not self._semantic_available():
            return []
        query_vec = self.embedder.embed_text(query)
        return self.vector_store.search(query_vec, k=k)

    def search_hybrid(self, query, k=5, warn=None):
        """Hybrid retrieval via Reciprocal Rank Fusion.

        Falls back to BM25 (with a warning) if the semantic index is
        unavailable.
        """
        corpus_size = max(len(self.bm25.chunks),
                          len(self.vector_store) if self.vector_store else 0)
        candidate_n = self._candidate_count(k, corpus_size)

        bm25_results = self.bm25.search(query, k=candidate_n)

        if not self._semantic_available():
            if warn:
                warn("Semantic index unavailable — falling back to BM25.")
            return self.bm25.search(query, k=k)

        query_vec = self.embedder.embed_text(query)
        semantic_results = self.vector_store.search(query_vec, k=candidate_n)

        fused = rrf_fuse([bm25_results, semantic_results], top_n=k)
        return fused

    def search(self, query, k=5, mode="bm25", file_type_filter=None,
               warn=None):
        """Unified entry point: 'bm25', 'semantic' or 'hybrid'."""
        if mode == "bm25":
            return self.search_bm25(query, k=k, file_type_filter=file_type_filter)
        if mode == "semantic":
            return self.search_semantic(query, k=k)
        if mode == "hybrid":
            return self.search_hybrid(query, k=k, warn=warn)
        raise ValueError(f"unknown retrieval mode: {mode}")
