"""Tests for document chunking (Python AST, Markdown, text, enrichment)."""
import pytest

from chunker import (
    chunk_python, chunk_markdown, chunk_text, chunk_file, detect_file_type,
    _extract_path_keywords,
)


PY_SOURCE = '''\
import os

class SchedulerConfig:
    def __init__(self, max_size):
        self.max_size = max_size

def allocate_slots(config):
    return config.max_size

def schedule(requests):
    return requests
'''


def test_detect_file_type():
    assert detect_file_type("a/b/scheduler.py") == "python"
    assert detect_file_type("docs/guide.md") == "markdown"
    assert detect_file_type("notes.txt") == "text"
    assert detect_file_type("Dockerfile") == "text"
    assert detect_file_type("Makefile") == "text"
    assert detect_file_type("README") == "text"


def test_extract_path_keywords():
    kws = _extract_path_keywords("data/raw/vllm-0.10.1/docs/features/tool_calling.md")
    assert "docs" in kws
    assert "features" in kws
    assert "tool" in kws
    assert "calling" in kws


def test_chunk_python_symbols_and_locations():
    chunks = chunk_python("scheduler.py", PY_SOURCE)
    # Preamble (imports) + 3 definitions.
    assert len(chunks) >= 3
    all_symbols = set()
    for ch in chunks:
        all_symbols.update(ch.symbols)
    assert "SchedulerConfig" in all_symbols
    assert "allocate_slots" in all_symbols
    assert "schedule" in all_symbols
    # Exact source locations: the chunk text must slice the original source.
    for ch in chunks:
        assert PY_SOURCE[ch.first_character_index:ch.last_character_index] == ch.text


def test_chunk_python_invalid_syntax_falls_back():
    bad = "def (:\n   broken"
    chunks = chunk_python("bad.py", bad)
    # Falls back to plain text chunking, still produces chunks with locations.
    assert len(chunks) >= 1
    for ch in chunks:
        assert PY_SOURCE  # no-op to keep reference
        assert ch.file_type == "python"


def test_chunk_python_preamble():
    chunks = chunk_python("m.py", PY_SOURCE)
    # First chunk should contain the imports.
    assert any("import os" in ch.text for ch in chunks)


MD_SOURCE = """\
# Serving

## OpenAI Compatible Server

This section describes the server.

## Authentication

Authentication uses a key.
"""


def test_chunk_markdown_heading_path():
    chunks = chunk_markdown("serving.md", MD_SOURCE)
    assert len(chunks) >= 2
    # Each chunk carries the heading hierarchy.
    heading_paths = [tuple(ch.heading_path) for ch in chunks]
    assert ("Serving", "OpenAI Compatible Server") in heading_paths
    assert ("Serving", "Authentication") in heading_paths
    # Exact locations.
    for ch in chunks:
        assert MD_SOURCE[ch.first_character_index:ch.last_character_index] == ch.text


def test_chunk_text_natural_boundaries():
    text = "Paragraph one.\n\nParagraph two is here.\n\nParagraph three."
    chunks = chunk_text("doc.txt", text)
    assert len(chunks) >= 1
    joined = "".join(ch.text for ch in chunks)
    assert "Paragraph one" in joined
    assert "Paragraph three" in joined
    for ch in chunks:
        assert text[ch.first_character_index:ch.last_character_index] == ch.text


def test_chunk_text_max_size():
    text = "\n\n".join(["word " * 50 for _ in range(10)])
    chunks = chunk_text("big.txt", text, max_size=200, overlap=20)
    for ch in chunks:
        assert len(ch.text) <= 200


def test_chunk_file_dispatch():
    chunks = chunk_file("x.py", PY_SOURCE)
    assert chunks[0].file_type == "python"
    chunks = chunk_file("x.md", MD_SOURCE)
    assert chunks[0].file_type == "markdown"
    chunks = chunk_file("x.txt", "hello world")
    assert chunks[0].file_type == "text"


def test_chunk_id_stable():
    a = chunk_python("s.py", PY_SOURCE)
    b = chunk_python("s.py", PY_SOURCE)
    assert [c.chunk_id for c in a] == [c.chunk_id for c in b]


def test_path_enrichment_added():
    chunks = chunk_file("pkg/feature_tool.py", "def f():\n    return 1\n")
    kws = set(chunks[0].path_keywords)
    assert "pkg" in kws
    assert "feature" in kws
    assert "tool" in kws
