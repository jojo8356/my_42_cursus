"""
Local HTTP API for the RAG project (bonus).

A lightweight FastAPI application exposing ``/search`` and ``/answer``. It
relies on the exact same retrieval pipeline used by the CLI and the terminal
interface, guaranteeing identical behaviour.

Run with:
    python3 rag.py api            # or: uvicorn src.api:app

FastAPI is an optional dependency; it is imported only when this module is
used, so the core project remains dependency-free.
"""
import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
if _HERE not in sys.path:
    sys.path.insert(0, _HERE)

from pydantic import BaseModel, Field  # noqa: E402

from config import RAGConfig  # noqa: E402
from indexer import Indexer  # noqa: E402
from retriever import Retriever  # noqa: E402
from generator import make_generator  # noqa: E402
from embedder import make_embedder  # noqa: E402
from vector_store import VectorStore  # noqa: E402
from bm25 import BM25Index  # noqa: E402

from fastapi import FastAPI  # noqa: E402

app = FastAPI(title="RAG Against the Machine", version="1.0.0")

_CONFIG_PATH = os.path.join(_HERE, "..", "data", "config.ini")


def _retriever():
    cfg = RAGConfig.load(_CONFIG_PATH)
    work_dir = cfg.work_dir
    if not os.path.isabs(work_dir):
        work_dir = os.path.join(os.path.dirname(_HERE), work_dir)
    embedder = make_embedder(cfg.embedder)
    bm25 = BM25Index()
    if os.path.exists(os.path.join(work_dir, "bm25.json")):
        bm25 = BM25Index.load(os.path.join(work_dir, "bm25.json"))
    vector_store = None
    if os.path.exists(os.path.join(work_dir, "vector.json")):
        vector_store = VectorStore.load(os.path.join(work_dir, "vector.json"))
    return Retriever(bm25=bm25, vector_store=vector_store, embedder=embedder)


class SearchRequest(BaseModel):
    query: str
    k: int = Field(default=5, ge=1, le=100)
    hybrid: bool = False


class AnswerRequest(BaseModel):
    query: str
    k: int = Field(default=5, ge=1, le=100)
    hybrid: bool = False


@app.get("/health")
def health():
    return {"status": "ok"}


@app.post("/search")
def search(req: SearchRequest):
    retriever = _retriever()
    results = retriever.search(req.query, k=req.k,
                               mode="hybrid" if req.hybrid else "bm25")
    return {"results": [r.to_dict() for r in results]}


@app.post("/answer")
def answer(req: AnswerRequest):
    retriever = _retriever()
    generator = make_generator(RAGConfig.load(_CONFIG_PATH).generator)
    results = retriever.search(req.query, k=req.k,
                               mode="hybrid" if req.hybrid else "bm25")
    ans = generator.generate(req.query, results)
    return ans.to_dict()
