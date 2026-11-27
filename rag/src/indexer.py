"""
Indexing pipeline: ingestion, incremental indexing and persistent caches.

The indexer scans a repository, discovers supported files, chunks them, and
builds the BM25 + semantic indices. It supports **incremental indexing**: a
repository manifest records each file's metadata (path, size, mtime, content
hash, chunking params); on a new run, unchanged files are reloaded from the
chunk cache instead of being reparsed, and only changed files are reprocessed.

Persistent caches:
- chunk cache: generated chunks persisted to disk and reloaded by chunk id;
- embedding cache: embeddings keyed by stable chunk id, reused across runs;
- query cache: BM25 retrieval results keyed by (query, k, expansion).

Every artifact is written atomically (temp file + ``os.replace``).
"""
import os
import hashlib
import json
import tempfile
from collections import defaultdict

from models import Chunk, ManifestEntry, stable_chunk_id
from chunker import chunk_file, detect_file_type, PYTHON_EXTENSIONS, \
    MARKDOWN_EXTENSIONS, TEXT_EXTENSIONS, TEXT_FILENAMES
from bm25 import BM25Index
from embedder import HashingEmbedder
from vector_store import VectorStore

SUPPORTED_EXTENSIONS = PYTHON_EXTENSIONS | MARKDOWN_EXTENSIONS | TEXT_EXTENSIONS


def _is_supported(file_path):
    lower = file_path.lower()
    ext = "." + lower.rsplit(".", 1)[-1] if "." in lower else ""
    if ext in SUPPORTED_EXTENSIONS:
        return True
    name = lower.rsplit("/", 1)[-1]
    return name in TEXT_FILENAMES


def hash_content(text):
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def _atomic_write_json(path, data):
    directory = os.path.dirname(os.path.abspath(path))
    os.makedirs(directory or ".", exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=directory or ".", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as fh:
            json.dump(data, fh)
            fh.flush()
            os.fsync(fh.fileno())
        os.replace(tmp, path)
    except Exception:
        if os.path.exists(tmp):
            os.remove(tmp)
        raise


def _atomic_write_text(path, text):
    directory = os.path.dirname(os.path.abspath(path))
    os.makedirs(directory or ".", exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=directory or ".", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as fh:
            fh.write(text)
            fh.flush()
            os.fsync(fh.fileno())
        os.replace(tmp, path)
    except Exception:
        if os.path.exists(tmp):
            os.remove(tmp)
        raise


class Indexer:
    """Builds and incrementally updates the BM25 + semantic indices."""

    def __init__(self, repo_path, work_dir, embedder=None,
                 max_chunk_size=2000, overlap=200,
                 bm25=None, vector_store=None):
        self.repo_path = repo_path
        self.work_dir = work_dir
        self.embedder = embedder or HashingEmbedder()
        self.max_chunk_size = max_chunk_size
        self.overlap = overlap
        self.bm25 = bm25 or BM25Index()
        self.vector_store = vector_store or VectorStore()

        self.manifest_path = os.path.join(work_dir, "manifest.json")
        self.chunks_path = os.path.join(work_dir, "chunks.json")
        self.embeddings_path = os.path.join(work_dir, "embeddings.json")
        self.query_cache_path = os.path.join(work_dir, "query_cache.json")

    # ------------------------------------------------------------------ discovery
    def discover_files(self):
        """Return the sorted list of supported file paths in the repository."""
        found = []
        for root, dirs, files in os.walk(self.repo_path):
            # Skip hidden / cache directories.
            dirs[:] = [d for d in dirs if not d.startswith(".")
                       and d not in ("__pycache__", "node_modules")]
            for name in files:
                full = os.path.join(root, name)
                if _is_supported(full):
                    found.append(os.path.relpath(full, self.repo_path))
        return sorted(found)

    def _file_metadata(self, file_path):
        full = os.path.join(self.repo_path, file_path)
        st = os.stat(full)
        with open(full, "r", encoding="utf-8", errors="replace") as fh:
            content = fh.read()
        return {
            "file_path": file_path,
            "size": st.st_size,
            "mtime": st.st_mtime,
            "content_hash": hash_content(content),
            "content": content,
        }

    # ------------------------------------------------------------------ manifest
    def load_manifest(self):
        if not os.path.exists(self.manifest_path):
            return {}
        with open(self.manifest_path, "r", encoding="utf-8") as fh:
            return json.load(fh)

    def save_manifest(self, manifest):
        _atomic_write_json(self.manifest_path, manifest)

    # ------------------------------------------------------------------ caches
    def load_chunk_cache(self):
        if not os.path.exists(self.chunks_path):
            return {}
        with open(self.chunks_path, "r", encoding="utf-8") as fh:
            data = json.load(fh)
        return {cid: Chunk.from_dict(d) for cid, d in data.items()}

    def save_chunk_cache(self, cache):
        _atomic_write_json(
            self.chunks_path, {cid: c.to_dict() for cid, c in cache.items()})

    def load_embedding_cache(self):
        if not os.path.exists(self.embeddings_path):
            return {}
        with open(self.embeddings_path, "r", encoding="utf-8") as fh:
            return json.load(fh)

    def save_embedding_cache(self, cache):
        _atomic_write_json(self.embeddings_path, cache)

    def load_query_cache(self):
        if not os.path.exists(self.query_cache_path):
            return {}
        with open(self.query_cache_path, "r", encoding="utf-8") as fh:
            return json.load(fh)

    def save_query_cache(self, cache):
        _atomic_write_json(self.query_cache_path, cache)

    # ------------------------------------------------------------------ indexing
    def _chunk_file(self, meta):
        """Chunk a single file's content into Chunk objects."""
        return chunk_file(
            meta["file_path"], meta["content"],
            max_size=self.max_chunk_size, overlap=self.overlap)

    def index(self, force=False, build_semantic=True, verbose=False):
        """Run (incremental) indexing. Returns a summary dict.

        Unchanged files are reloaded from the chunk cache; only added or
        modified files are reparsed. Embeddings are reused from the cache.
        """
        os.makedirs(self.work_dir, exist_ok=True)
        manifest = {} if force else self.load_manifest()
        chunk_cache = {} if force else self.load_chunk_cache()
        emb_cache = {} if force else self.load_embedding_cache()

        discovered = self.discover_files()
        new_manifest = {}
        added = modified = unchanged = removed = 0
        all_chunks = []
        new_chunks_count = 0
        reused_chunks_count = 0
        embeddings_generated = 0
        embeddings_reused = 0

        for file_path in discovered:
            meta = self._file_metadata(file_path)
            prev = manifest.get(file_path)
            entry = ManifestEntry(
                file_path=file_path,
                size=meta["size"],
                mtime=meta["mtime"],
                content_hash=meta["content_hash"],
                chunking_params={
                    "max_size": self.max_chunk_size,
                    "overlap": self.overlap,
                },
            )
            new_manifest[file_path] = entry.to_dict()

            if prev and prev.get("content_hash") == meta["content_hash"] \
                    and prev.get("chunking_params") == entry.chunking_params:
                # Unchanged: reuse cached chunks for this file.
                unchanged += 1
                file_chunks = [
                    c for c in chunk_cache.values()
                    if c.file_path == file_path
                ]
                reused_chunks_count += len(file_chunks)
            else:
                if prev:
                    modified += 1
                else:
                    added += 1
                file_chunks = self._chunk_file(meta)
                new_chunks_count += len(file_chunks)
                # Update the chunk cache for this file.
                for c in file_chunks:
                    chunk_cache[c.chunk_id] = c
            all_chunks.extend(file_chunks)

        # Detect removed files (present in old manifest, absent now).
        for file_path in manifest:
            if file_path not in new_manifest:
                removed += 1
                # Drop cached chunks for removed files.
                chunk_cache = {
                    cid: c for cid, c in chunk_cache.items()
                    if c.file_path != file_path
                }

        # Build the BM25 index from the (possibly reused) chunk set.
        self.bm25 = BM25Index()
        self.bm25.add_all(list(chunk_cache.values()))

        # Build / reuse the semantic vector store.
        if build_semantic:
            self.vector_store = VectorStore()
            for chunk in chunk_cache.values():
                if chunk.chunk_id in emb_cache:
                    embeddings_reused += 1
                    emb = emb_cache[chunk.chunk_id]
                else:
                    emb = self.embedder.embed_text(chunk.text)
                    emb_cache[chunk.chunk_id] = emb
                    embeddings_generated += 1
                self.vector_store.add(chunk, emb)
            self.save_embedding_cache(emb_cache)

        # Persist caches and manifest.
        self.save_chunk_cache(chunk_cache)
        self.save_manifest(new_manifest)

        summary = {
            "files_added": added,
            "files_modified": modified,
            "files_unchanged": unchanged,
            "files_removed": removed,
            "chunks_total": len(chunk_cache),
            "chunks_new": new_chunks_count,
            "chunks_reused": reused_chunks_count,
            "embeddings_generated": embeddings_generated,
            "embeddings_reused": embeddings_reused,
            "semantic_built": build_semantic,
        }
        return summary
