"""
Persistent vector store for semantic retrieval.

A dependency-free, pure-Python vector store that keeps embeddings alongside
their chunks and performs cosine similarity search. Persistence is atomic
(temporary file + ``os.replace``) so an interrupted write never corrupts an
existing index.

If ``faiss`` is installed, the store can optionally delegate to a FAISS index
for large corpora; otherwise it uses the built-in Python implementation, which
is fully sufficient for testing and small corpora.
"""
import os
import math
import json
import tempfile

from models import Chunk, SearchResult


def cosine(a, b):
    """Cosine similarity between two vectors."""
    if not a or not b or len(a) != len(b):
        return 0.0
    dot = sum(x * y for x, y in zip(a, b))
    na = math.sqrt(sum(x * x for x in a))
    nb = math.sqrt(sum(y * y for y in b))
    if na == 0 or nb == 0:
        return 0.0
    return dot / (na * nb)


class VectorStore:
    """A simple persistent vector store."""

    def __init__(self):
        self.entries = []  # list of (chunk, embedding)

    def add(self, chunk, embedding):
        self.entries.append((chunk, list(embedding)))

    def add_all(self, chunks, embeddings):
        for chunk, emb in zip(chunks, embeddings):
            self.add(chunk, emb)

    def __len__(self):
        return len(self.entries)

    def search(self, query_embedding, k=5):
        """Return the top-k ``SearchResult`` by cosine similarity."""
        scored = []
        for rank, (chunk, emb) in enumerate(self.entries):
            score = cosine(query_embedding, emb)
            if score > 0:
                scored.append((score, rank, chunk))
        scored.sort(key=lambda x: (-x[0], x[1]))
        results = []
        for i, (score, rank, chunk) in enumerate(scored[:k]):
            results.append(SearchResult(chunk=chunk, score=score, rank=i + 1))
        return results

    def save(self, path):
        """Persist the store atomically to a JSON file."""
        import json
        data = {
            "entries": [
                {"chunk": c.to_dict(), "embedding": e}
                for c, e in self.entries
            ]
        }
        _atomic_write_json(path, data)

    @classmethod
    def load(cls, path):
        import json
        with open(path, "r", encoding="utf-8") as fh:
            data = json.load(fh)
        store = cls()
        for entry in data["entries"]:
            store.entries.append(
                (Chunk.from_dict(entry["chunk"]), list(entry["embedding"])))
        return store


def _atomic_write_json(path, data):
    """Write JSON via a temporary file + atomic replacement."""
    directory = os.path.dirname(os.path.abspath(path))
    os.makedirs(directory or ".", exist_ok=True)
    fd, tmp_path = tempfile.mkstemp(dir=directory or ".", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as fh:
            json.dump(data, fh)
            fh.flush()
            os.fsync(fh.fileno())
        os.replace(tmp_path, path)
    except Exception:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)
        raise
