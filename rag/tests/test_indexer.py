"""Tests for the indexing pipeline (ingestion, incremental, caches)."""
import os
import time

import pytest

from indexer import Indexer, hash_content
from models import Chunk


def _write(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(content)


@pytest.fixture
def repo(tmp_path):
    """A tiny repository with a python file and a markdown file."""
    repo_dir = tmp_path / "repo"
    _write(str(repo_dir / "scheduler.py"),
           "def allocate_slots(config):\n    return config.max_size\n\n"
           "def schedule(requests):\n    return requests\n")
    _write(str(repo_dir / "docs.md"),
           "# Serving\n\n## Scheduler\n\nThe scheduler allocates slots.\n")
    return str(repo_dir)


def test_discover_files(repo, tmp_path):
    idx = Indexer(repo, str(tmp_path / "work"))
    files = idx.discover_files()
    assert "scheduler.py" in files
    assert "docs.md" in files


def test_index_builds_bm25_and_semantic(repo, tmp_path):
    idx = Indexer(repo, str(tmp_path / "work"))
    summary = idx.index(build_semantic=True)
    assert summary["chunks_total"] > 0
    assert summary["files_added"] == 2
    assert summary["semantic_built"] is True
    assert len(idx.bm25.chunks) > 0
    assert len(idx.vector_store) > 0


def test_index_persists_manifest_and_caches(repo, tmp_path):
    work = str(tmp_path / "work")
    idx = Indexer(repo, work)
    idx.index(build_semantic=True)
    assert os.path.exists(os.path.join(work, "manifest.json"))
    assert os.path.exists(os.path.join(work, "chunks.json"))
    assert os.path.exists(os.path.join(work, "embeddings.json"))


def test_incremental_reuses_unchanged(repo, tmp_path):
    work = str(tmp_path / "work")
    idx = Indexer(repo, work)
    summary1 = idx.index(build_semantic=True)
    # Second run with no changes: all files unchanged, chunks reused.
    summary2 = idx.index(build_semantic=True)
    assert summary2["files_unchanged"] == 2
    assert summary2["chunks_new"] == 0
    assert summary2["chunks_reused"] == summary1["chunks_total"]


def test_incremental_detects_modified(repo, tmp_path):
    work = str(tmp_path / "work")
    idx = Indexer(repo, work)
    idx.index()
    # Modify a file.
    _write(str(os.path.join(repo, "scheduler.py")),
           "def allocate_slots(config):\n    return config.max_size\n\n"
           "def schedule(requests):\n    return sorted(requests)\n")
    summary = idx.index()
    assert summary["files_modified"] == 1
    assert summary["files_unchanged"] == 1


def test_incremental_detects_added_and_removed(repo, tmp_path):
    work = str(tmp_path / "work")
    idx = Indexer(repo, work)
    idx.index()
    _write(str(os.path.join(repo, "new.txt")), "a new plain text note")
    os.remove(os.path.join(repo, "docs.md"))
    summary = idx.index()
    assert summary["files_added"] == 1
    assert summary["files_removed"] == 1


def test_embedding_cache_reused(repo, tmp_path):
    work = str(tmp_path / "work")
    idx = Indexer(repo, work)
    s1 = idx.index(build_semantic=True)
    s2 = idx.index(build_semantic=True)
    # Second run reuses all embeddings.
    assert s2["embeddings_generated"] == 0
    assert s2["embeddings_reused"] == s1["embeddings_generated"]


def test_force_rebuild(repo, tmp_path):
    work = str(tmp_path / "work")
    idx = Indexer(repo, work)
    idx.index()
    s = idx.index(force=True)
    assert s["files_added"] == 2  # everything rebuilt
    assert s["files_unchanged"] == 0


def test_hash_content_stable():
    assert hash_content("abc") == hash_content("abc")
    assert hash_content("abc") != hash_content("abd")


def test_manifest_entry_roundtrip(tmp_path):
    from models import ManifestEntry
    entry = ManifestEntry("f.py", 10, 1.0, "hash123",
                          chunking_params={"max_size": 2000})
    d = entry.to_dict()
    entry2 = ManifestEntry.from_dict(d)
    assert entry2.file_path == "f.py"
    assert entry2.content_hash == "hash123"
