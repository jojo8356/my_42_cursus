"""Tests for the answer generator (grounding, anti-hallucination)."""
import pytest

from models import Chunk, SearchResult, stable_chunk_id
from generator import TemplateGenerator, build_prompt, build_context, make_generator


def _result(text, file_path="f.md", score=1.0):
    chunk = Chunk(file_path, text, 0, len(text), "markdown",
                  chunk_id=stable_chunk_id(file_path, 0, len(text), text))
    return SearchResult(chunk=chunk, score=score, rank=1)


def test_build_context_contains_sources():
    results = [_result("Scheduler allocates slots.")]
    ctx = build_context(results)
    assert "Scheduler allocates slots." in ctx
    assert "f.md" in ctx


def test_build_prompt_restricts_to_context():
    prompt = build_prompt("How does it work?", [_result("The scheduler works.")])
    assert "ONLY the provided context" in prompt
    assert "How does it work?" in prompt


def test_template_generator_grounded():
    gen = TemplateGenerator()
    results = [_result("The scheduler allocates slots for requests.", score=2.0)]
    ans = gen.generate("How does the scheduler work?", results)
    assert "scheduler" in ans.text.lower()
    assert ans.confidence == 2.0
    assert ans.sources == results


def test_template_generator_no_results():
    gen = TemplateGenerator()
    ans = gen.generate("question", [])
    assert ans.confidence == 0.0
    assert "No relevant information" in ans.text


def test_template_generator_low_score_antihallucination():
    gen = TemplateGenerator(min_score=5.0)
    results = [_result("some text", score=0.1)]
    ans = gen.generate("question", results)
    assert "No sufficiently relevant information" in ans.text


def test_template_generator_extracts_relevant_sentence():
    gen = TemplateGenerator()
    results = [_result(
        "The cache stores embeddings. The scheduler allocates slots. "
        "Authentication validates keys.", score=2.0)]
    ans = gen.generate("scheduler slots", results)
    # The sentence mentioning scheduler should be selected.
    assert "scheduler" in ans.text.lower()


def test_make_generator_factory():
    assert isinstance(make_generator("template"), TemplateGenerator)
    with pytest.raises(ValueError):
        make_generator("unknown")
