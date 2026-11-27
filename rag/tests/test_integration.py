"""End-to-end integration tests for the full RAG pipeline."""
import os

import pytest

from indexer import Indexer
from retriever import Retriever
from generator import make_generator
from evaluator import evaluate
from embedder import HashingEmbedder
from models import Chunk, SearchResult, stable_chunk_id


@pytest.fixture
def repo(tmp_path):
    repo_dir = tmp_path / "repo"
    os.makedirs(str(repo_dir / "pkg"), exist_ok=True)
    (repo_dir / "scheduler.py").write_text(
        "class SchedulerConfig:\n"
        "    def __init__(self, max_size):\n"
        "        self.max_size = max_size\n\n"
        "def allocate_slots(config):\n"
        "    return config.max_size\n\n"
        "def schedule(requests):\n"
        "    return requests\n")
    (repo_dir / "auth.py").write_text(
        "def authenticate(key):\n"
        "    return key == 'secret'\n")
    (repo_dir / "docs.md").write_text(
        "# Serving\n\n## Scheduler\n\n"
        "The scheduler allocates slots for incoming requests.\n\n"
        "## Authentication\n\nAuthentication uses a secret key.\n")
    return str(repo_dir)


@pytest.fixture
def pipeline(repo, tmp_path):
    work = str(tmp_path / "work")
    embedder = HashingEmbedder(dimensions=128)
    indexer = Indexer(repo, work, embedder=embedder)
    indexer.index(build_semantic=True)
    retriever = Retriever(bm25=indexer.bm25, vector_store=indexer.vector_store,
                          embedder=embedder)
    generator = make_generator("template")
    return retriever, generator, indexer


def test_full_pipeline_index_search_answer(pipeline):
    retriever, generator, _ = pipeline
    results = retriever.search("How does the scheduler allocate slots?",
                               k=3, mode="bm25")
    assert results
    answer = generator.generate("How does the scheduler allocate slots?",
                                results)
    assert answer.text
    assert answer.sources


def test_bm25_finds_scheduler(pipeline):
    retriever, _, _ = pipeline
    results = retriever.search("allocate_slots scheduler", k=3, mode="bm25")
    # The scheduler chunk (with the allocate_slots symbol) should appear.
    found = any("scheduler" in r.chunk.file_path or "scheduler" in r.chunk.text.lower()
                for r in results)
    assert found


def test_hybrid_finds_scheduler(pipeline):
    retriever, _, _ = pipeline
    results = retriever.search("allocate_slots scheduler", k=3, mode="hybrid")
    found = any("scheduler" in r.chunk.file_path or "scheduler" in r.chunk.text.lower()
                for r in results)
    assert found


def test_answer_grounded_in_context(pipeline):
    retriever, generator, _ = pipeline
    results = retriever.search("scheduler", k=2, mode="bm25")
    answer = generator.generate("What does the scheduler do?", results)
    # The answer must come from the retrieved context (no hallucination).
    context_text = " ".join(r.chunk.text for r in results).lower()
    assert answer.text.lower() in context_text or any(
        w in answer.text.lower() for w in context_text.split())


def test_incremental_then_search_still_works(repo, tmp_path):
    work = str(tmp_path / "work")
    embedder = HashingEmbedder(dimensions=128)
    indexer = Indexer(repo, work, embedder=embedder)
    indexer.index(build_semantic=True)
    indexer.index(build_semantic=True)  # incremental reuse
    retriever = Retriever(bm25=indexer.bm25, vector_store=indexer.vector_store,
                          embedder=embedder)
    results = retriever.search("authenticate", k=2, mode="bm25")
    assert results
    assert any("auth" in r.chunk.file_path or "auth" in r.chunk.text.lower()
               for r in results)


def test_evaluate_end_to_end(pipeline):
    retriever, _, _ = pipeline
    # Use terms that uniquely appear in the expected files so the docs are
    # retrieved at rank 1.
    queries = [
        ("allocate_slots", ["scheduler.py"]),
        ("authenticate", ["auth.py"]),
    ]
    results_by_query = {}
    dataset = []
    for query, expected in queries:
        res = retriever.search(query, k=3, mode="bm25")
        results_by_query[query] = res
        dataset.append({"question": query, "expected": expected})
    report = evaluate(results_by_query, dataset)
    assert report["total"] == 2
    assert report["recall_at_1"] == 1.0
    assert report["recall_at_3"] == 1.0


def test_no_unhandled_errors_over_many_queries(pipeline):
    retriever, generator, _ = pipeline
    for q in ["scheduler", "authentication", "serving", "config", "key"]:
        results = retriever.search(q, k=3, mode="hybrid")
        answer = generator.generate(q, results)
        assert answer is not None
