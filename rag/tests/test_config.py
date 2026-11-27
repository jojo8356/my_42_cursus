"""Tests for the persistent configuration."""
import pytest

from config import RAGConfig, DEFAULTS


def test_defaults():
    cfg = RAGConfig()
    assert cfg.chunk_max_size == 2000
    assert cfg.chunk_overlap == 200
    assert cfg.top_k == 5
    assert cfg.hybrid is False
    assert cfg.embedder == "hashing"


def test_save_load_roundtrip(tmp_path):
    cfg = RAGConfig(repo_path="/tmp/repo", top_k=7, hybrid="true")
    path = str(tmp_path / "config.ini")
    cfg.save(path)
    cfg2 = RAGConfig.load(path)
    assert cfg2.repo_path == "/tmp/repo"
    assert cfg2.top_k == 7
    assert cfg2.hybrid is True


def test_load_missing_falls_back(tmp_path):
    cfg = RAGConfig.load(str(tmp_path / "nope.ini"))
    assert cfg.chunk_max_size == 2000


def test_invalid_values_fall_back():
    cfg = RAGConfig(chunk_max_size="abc", top_k=-5, chunk_overlap="999999")
    assert cfg.chunk_max_size == 2000
    assert cfg.top_k == 5
    assert cfg.chunk_overlap == 200


def test_to_dict():
    cfg = RAGConfig(top_k=3)
    d = cfg.to_dict()
    assert d["top_k"] == 3
    assert "repo_path" in d
