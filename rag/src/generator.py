"""
Answer generation for the RAG pipeline.

The generator turns retrieved chunks into a grounded answer. The default
``TemplateGenerator`` is fully local and deterministic: it builds a restricted
context and produces an extractive answer that only uses retrieved information,
so it cannot hallucinate. An ``OpenAIGenerator`` is provided for model-backed
answers when an API key is available.

Anti-hallucination mechanisms:
- a restricted prompt instructing the model to answer only from the context;
- a limited context (only the top chunks);
- an optional BM25 relevance check: if the best retrieval score is too weak,
  a dedicated "no relevant information" response is returned instead.
"""
import re

from models import Answer, SearchResult


# Threshold below which a BM25-only answer is considered ungrounded.
DEFAULT_MIN_SCORE = 0.5


class BaseGenerator:
    """Interface for answer generation."""

    def generate(self, query, results):
        raise NotImplementedError


def build_context(results):
    """Assemble retrieved chunks into a structured context string."""
    parts = []
    for i, r in enumerate(results, start=1):
        src = r.chunk.file_path
        parts.append(
            f"[Source {i}] {src} "
            f"(chars {r.chunk.first_character_index}-"
            f"{r.chunk.last_character_index}):\n{r.chunk.text}")
    return "\n\n".join(parts)


def build_prompt(query, results):
    """Restricted prompt: answer only from the supplied context."""
    context = build_context(results)
    return (
        "Answer the question using ONLY the provided context.\n"
        "If the context does not contain the answer, say you do not know.\n\n"
        f"Context:\n{context}\n\nQuestion: {query}\nAnswer:")


class TemplateGenerator(BaseGenerator):
    """Local, deterministic, extractive generator (no external model)."""

    def __init__(self, min_score=DEFAULT_MIN_SCORE):
        self.min_score = min_score

    def _extractive_answer(self, query, results):
        """Pick the most relevant sentences from the retrieved chunks."""
        query_terms = set(re.findall(r"[A-Za-z0-9_]+", query.lower()))
        scored_sentences = []
        for r in results:
            sentences = re.split(r"(?<=[.!?])\s+", r.chunk.text.strip())
            for sent in sentences:
                sent_l = sent.lower()
                overlap = sum(1 for t in query_terms if t in sent_l)
                if overlap > 0:
                    scored_sentences.append((overlap, sent))
        scored_sentences.sort(key=lambda x: -x[0])
        picked = []
        seen = set()
        for _, sent in scored_sentences:
            key = sent.strip()
            if key and key not in seen:
                picked.append(sent.strip())
                seen.add(key)
            if len(picked) >= 3:
                break
        if not picked:
            # No sentence matched: fall back to the first chunk's opening.
            if results:
                picked = [results[0].chunk.text.strip().splitlines()[0]]
        return " ".join(picked)

    def generate(self, query, results):
        if not results:
            return Answer(text="No relevant information was found.",
                          sources=[], confidence=0.0)

        # BM25 relevance check (only meaningful for lexical scores).
        max_score = max(r.score for r in results)
        if max_score < self.min_score:
            return Answer(
                text="No sufficiently relevant information was found.",
                sources=results, confidence=max_score)

        answer_text = self._extractive_answer(query, results)
        return Answer(text=answer_text, sources=results,
                      confidence=max_score)


class OpenAIGenerator(BaseGenerator):
    """Generator backed by an OpenAI-compatible chat API (optional)."""

    def __init__(self, model="gpt-4o-mini", api_key=None, temperature=0.0):
        import os
        self.model = model
        self.api_key = api_key or os.environ.get("OPENAI_API_KEY")
        self.temperature = temperature
        if not self.api_key:
            raise ValueError("OPENAI_API_KEY is required for OpenAIGenerator")

    def generate(self, query, results):
        import openai
        prompt = build_prompt(query, results)
        client = openai.OpenAI(api_key=self.api_key)
        resp = client.chat.completions.create(
            model=self.model,
            messages=[{"role": "user", "content": prompt}],
            temperature=self.temperature,
        )
        text = resp.choices[0].message.content
        return Answer(text=text, sources=results,
                      confidence=max((r.score for r in results), default=0.0))


def make_generator(kind="template", **kwargs):
    """Factory: build a generator by name ('template' or 'openai')."""
    if kind == "template":
        return TemplateGenerator(**kwargs)
    if kind == "openai":
        return OpenAIGenerator(**kwargs)
    raise ValueError(f"unknown generator kind: {kind}")
