#!/usr/bin/env python3
"""
RAG — RAG Against the Machine — command-line entry point.

Usage:
    python3 rag.py index [--hybrid] [--force]
    python3 rag.py search "query" [--k 5] [--hybrid]
    python3 rag.py answer "query" [--k 5] [--hybrid]
    python3 rag.py evaluate --results results.json --dataset dataset.json
    python3 rag.py api
    python3 rag.py interface

The CLI orchestrates the same pipeline used by the HTTP API and the terminal
interface, guaranteeing identical behaviour across all front-ends.
"""
import argparse
import json
import os
import sys

# Make src/ importable when running this file directly.
_HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(_HERE, "src"))

from config import RAGConfig  # noqa: E402
from indexer import Indexer  # noqa: E402
from retriever import Retriever  # noqa: E402
from generator import make_generator  # noqa: E402
from embedder import make_embedder  # noqa: E402
from evaluator import evaluate  # noqa: E402
from vector_store import VectorStore  # noqa: E402
from bm25 import BM25Index  # noqa: E402


def _load_config(args):
    path = args.config or os.path.join(_HERE, "data", "config.ini")
    cfg = RAGConfig.load(path)
    # Command-line overrides.
    if args.repo:
        cfg._data["repo_path"] = args.repo
    if args.work_dir:
        cfg._data["work_dir"] = args.work_dir
    return cfg


def _build_pipeline(cfg, embedder_kind=None, build_semantic=False):
    """Build a Retriever + generator from the config (loading persisted data)."""
    work_dir = cfg.work_dir
    if not os.path.isabs(work_dir):
        work_dir = os.path.join(_HERE, work_dir)

    embedder = make_embedder(embedder_kind or cfg.embedder)
    bm25 = None
    if os.path.exists(os.path.join(work_dir, "chunks.json")):
        bm25 = BM25Index.load(os.path.join(work_dir, "bm25.json")) \
            if os.path.exists(os.path.join(work_dir, "bm25.json")) else None
    vector_store = None
    if build_semantic and os.path.exists(os.path.join(work_dir, "vector.json")):
        vector_store = VectorStore.load(os.path.join(work_dir, "vector.json"))

    retriever = Retriever(bm25=bm25 or BM25Index(),
                          vector_store=vector_store, embedder=embedder)
    generator = make_generator(cfg.generator)
    return retriever, generator


def _ensure_index(cfg, hybrid=False, force=False):
    """Run indexing if needed and return the built retriever."""
    work_dir = cfg.work_dir
    if not os.path.isabs(work_dir):
        work_dir = os.path.join(_HERE, work_dir)
    embedder = make_embedder(cfg.embedder)
    indexer = Indexer(
        repo_path=cfg.repo_path, work_dir=work_dir, embedder=embedder,
        max_chunk_size=cfg.chunk_max_size, overlap=cfg.chunk_overlap,
    )
    summary = indexer.index(force=force, build_semantic=hybrid)
    # Persist the built BM25 + vector store for the front-ends to reuse.
    indexer.bm25.save(os.path.join(work_dir, "bm25.json"))
    if hybrid:
        indexer.vector_store.save(os.path.join(work_dir, "vector.json"))
    return Retriever(bm25=indexer.bm25,
                     vector_store=indexer.vector_store, embedder=embedder), summary


def cmd_index(args):
    cfg = _load_config(args)
    retriever, summary = _ensure_index(cfg, hybrid=args.hybrid, force=args.force)
    print(json.dumps(summary, indent=2, ensure_ascii=False))
    return 0


def cmd_search(args):
    cfg = _load_config(args)
    retriever, _ = _ensure_index(cfg, hybrid=args.hybrid)
    mode = "hybrid" if args.hybrid else "bm25"
    results = retriever.search(args.query, k=args.k, mode=mode,
                               warn=lambda m: print("WARNING:", m))
    output = [r.to_dict() for r in results]
    print(json.dumps(output, indent=2, ensure_ascii=False))
    return 0


def cmd_answer(args):
    cfg = _load_config(args)
    retriever, _ = _ensure_index(cfg, hybrid=args.hybrid)
    generator = make_generator(cfg.generator)
    mode = "hybrid" if args.hybrid else "bm25"
    results = retriever.search(args.query, k=args.k, mode=mode,
                               warn=lambda m: print("WARNING:", m))
    answer = generator.generate(args.query, results)
    print(json.dumps(answer.to_dict(), indent=2, ensure_ascii=False))
    return 0


def cmd_evaluate(args):
    with open(args.results, "r", encoding="utf-8") as fh:
        results_by_query = json.load(fh)
    with open(args.dataset, "r", encoding="utf-8") as fh:
        dataset = json.load(fh)
    # results_by_query: {question: [ {file_path, ...} ]} -> convert to
    # SearchResult-like objects.
    from models import SearchResult, Chunk
    converted = {}
    for question, items in results_by_query.items():
        converted[question] = [
            SearchResult(
                chunk=Chunk(
                    file_path=item["file_path"],
                    text=item.get("text", ""),
                    first_character_index=item.get("first_character_index", 0),
                    last_character_index=item.get("last_character_index", 0),
                    file_type=item.get("file_type", "text"),
                ),
                score=item.get("score", 0.0),
                rank=item.get("rank", 0),
            )
            for item in items
        ]
    report = evaluate(converted, dataset)
    print(json.dumps(report, indent=2, ensure_ascii=False))
    return 0


def cmd_api(args):
    os.environ.setdefault("PYTHONPATH", os.path.join(_HERE, "src"))
    import uvicorn
    from api import app  # noqa: E402
    uvicorn.run(app, host="0.0.0.0", port=args.port)
    return 0


def cmd_interface(args):
    cfg = _load_config(args)
    import interface  # noqa: E402
    return interface.run(cfg)


def build_parser():
    parser = argparse.ArgumentParser(
        prog="rag.py", description="RAG Against the Machine")
    parser.add_argument("--config", help="config.ini path")
    parser.add_argument("--repo", help="repository path override")
    parser.add_argument("--work-dir", dest="work_dir", help="work dir override")
    sub = parser.add_subparsers(dest="command", required=True)

    p_idx = sub.add_parser("index", help="build / incrementally update indices")
    p_idx.add_argument("--hybrid", action="store_true",
                       help="also build the semantic index")
    p_idx.add_argument("--force", action="store_true",
                       help="ignore manifest and rebuild everything")
    p_idx.set_defaults(func=cmd_index)

    p_search = sub.add_parser("search", help="retrieve top-k chunks")
    p_search.add_argument("query")
    p_search.add_argument("--k", type=int, default=None)
    p_search.add_argument("--hybrid", action="store_true")
    p_search.set_defaults(func=cmd_search)

    p_answer = sub.add_parser("answer", help="generate a grounded answer")
    p_answer.add_argument("query")
    p_answer.add_argument("--k", type=int, default=None)
    p_answer.add_argument("--hybrid", action="store_true")
    p_answer.set_defaults(func=cmd_answer)

    p_eval = sub.add_parser("evaluate", help="compute Recall@k")
    p_eval.add_argument("--results", required=True)
    p_eval.add_argument("--dataset", required=True)
    p_eval.set_defaults(func=cmd_evaluate)

    p_api = sub.add_parser("api", help="run the FastAPI HTTP server")
    p_api.add_argument("--port", type=int, default=8000)
    p_api.set_defaults(func=cmd_api)

    p_iface = sub.add_parser("interface", help="interactive terminal interface")
    p_iface.set_defaults(func=cmd_interface)

    return parser


def main(argv=None):
    parser = build_parser()
    args = parser.parse_args(argv)
    # Resolve top_k default from config if not given.
    if getattr(args, "k", None) is None and args.command in ("search", "answer"):
        cfg = _load_config(args)
        args.k = cfg.top_k
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
