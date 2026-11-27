"""
BM25 lexical retrieval engine.

A pure, dependency-free implementation of the Okapi BM25 ranking function,
plus the optional ranking bonuses described in the subject:

- **query expansion**: selected concepts can be expanded with a smaller weight;
- **AST symbol bonus**: chunks whose extracted symbols match the query get a
  small score bonus;
- **documentation bonus**: a configurable bonus applied to documentation
  chunks.

Tokenisation is lowercase word tokenisation with an optional stopword filter.
All randomness is injected, so behaviour is deterministic and testable.
"""
import math
import re
from collections import defaultdict, Counter

from models import SearchResult

# Default BM25 parameters.
DEFAULT_K1 = 1.5
DEFAULT_B = 0.75

# Minimal English stopwords (kept small to stay language-agnostic-ish).
DEFAULT_STOPWORDS = {
    "a", "an", "the", "and", "or", "of", "to", "in", "on", "for", "is",
    "are", "how", "does", "do", "what", "which", "with", "it", "its",
    "this", "that", "as", "at", "by", "be", "from",
}

_TOKEN_RE = re.compile(r"[A-Za-z0-9_]+")


def tokenize(text, stopwords=None):
    """Tokenise text into lowercase word tokens (optionally dropping stopwords)."""
    stop = DEFAULT_STOPWORDS if stopwords is None else stopwords
    tokens = []
    for match in _TOKEN_RE.finditer(text):
        tok = match.group(0).lower()
        if stop and tok in stop:
            continue
        tokens.append(tok)
    return tokens


class BM25Index:
    """An in-memory BM25 index over a corpus of chunks."""

    def __init__(self, k1=DEFAULT_K1, b=DEFAULT_B, stopwords=None,
                 symbol_bonus=2.0, docs_bonus=1.5, expansions=None):
        self.k1 = k1
        self.b = b
        self.stopwords = stopwords
        self.symbol_bonus = symbol_bonus
        self.docs_bonus = docs_bonus
        self.expansions = expansions or {}  # term -> [expanded terms]

        self.chunks = []                 # index 0..n-1
        self.doc_tokens = []             # list[list[str]]
        self.doc_lengths = []            # list[int]
        self.tf = []                     # list[Counter]
        self.df = Counter()              # term -> document frequency
        self.avg_length = 0.0
        self.N = 0
        self._build_state = False

    # ------------------------------------------------------------------ indexing
    def add(self, chunk):
        """Add a single chunk to the index."""
        tokens = tokenize(chunk.text, self.stopwords)
        # Enrich with symbols and path keywords for better lexical matching.
        for sym in chunk.symbols:
            for tok in tokenize(sym, None):
                if tok not in tokens:
                    tokens.append(tok)
        for kw in chunk.path_keywords:
            if kw not in tokens:
                tokens.append(kw)
        self.chunks.append(chunk)
        self.doc_tokens.append(tokens)
        self.doc_lengths.append(len(tokens))
        tf = Counter(tokens)
        self.tf.append(tf)
        for term in tf:
            self.df[term] += 1
        self.N += 1
        self._build_state = False

    def add_all(self, chunks):
        for chunk in chunks:
            self.add(chunk)
        return self

    def _finalize(self):
        if self._build_state:
            return
        self.avg_length = (sum(self.doc_lengths) / self.N) if self.N else 0.0
        self._build_state = True

    def _idf(self, term):
        # Okapi BM25 idf (with +0.5 smoothing, non-negative).
        df = self.df.get(term, 0)
        if df == 0:
            return 0.0
        idf = math.log(1 + (self.N - df + 0.5) / (df + 0.5))
        return max(idf, 0.0)

    def _score(self, doc_idx, query_terms):
        """BM25 score of document ``doc_idx`` for the query terms."""
        self._finalize()
        if not self.doc_lengths:
            return 0.0
        tf = self.tf[doc_idx]
        dl = self.doc_lengths[doc_idx]
        score = 0.0
        for term in query_terms:
            if term not in tf:
                continue
            f = tf[term]
            idf = self._idf(term)
            denom = f + self.k1 * (1 - self.b + self.b * dl / self.avg_length)
            score += idf * (f * (self.k1 + 1)) / denom
        return score

    # ------------------------------------------------------------------ retrieval
    def _expanded_query_terms(self, query_text):
        """Return the primary query terms plus lightly-weighted expansions.

        The original query remains dominant: expansions are appended with a
        smaller multiplier applied separately during scoring.
        """
        primary = tokenize(query_text, self.stopwords)
        expanded = []
        for term in primary:
            if term in self.expansions:
                for exp in self.expansions[term]:
                    for tok in tokenize(exp, None):
                        if tok not in expanded:
                            expanded.append(tok)
        return primary, expanded

    def search(self, query, k=5, file_type_filter=None, apply_bonus=True):
        """Return the top-k ``SearchResult`` for a query."""
        primary, expanded = self._expanded_query_terms(query)
        if not primary and not expanded:
            return []

        scores = []
        for idx in range(self.N):
            if file_type_filter and self.chunks[idx].file_type != file_type_filter:
                continue
            base = self._score(idx, primary)
            # Expansion terms contribute with a smaller weight.
            if expanded:
                exp_score = self._score(idx, expanded)
                base += 0.5 * exp_score
            if apply_bonus:
                chunk = self.chunks[idx]
                # AST symbol bonus.
                query_tokens = set(tokenize(query, None))
                if any(sym.lower() in query_tokens for sym in chunk.symbols):
                    base += self.symbol_bonus
                # Documentation bonus (markdown / rst docs).
                if chunk.file_type == "markdown":
                    base += self.docs_bonus
            if base > 0:
                scores.append((base, idx))

        scores.sort(key=lambda x: (-x[0], x[1]))
        results = []
        for rank, (score, idx) in enumerate(scores[:k]):
            results.append(SearchResult(chunk=self.chunks[idx], score=score,
                                        rank=rank + 1))
        return results

    def save(self, path):
        """Persist the index (chunks + tf + df) to a JSON file."""
        import json
        data = {
            "chunks": [c.to_dict() for c in self.chunks],
            "df": dict(self.df),
            "doc_lengths": self.doc_lengths,
            "k1": self.k1,
            "b": self.b,
            "symbol_bonus": self.symbol_bonus,
            "docs_bonus": self.docs_bonus,
            "expansions": self.expansions,
        }
        with open(path, "w", encoding="utf-8") as fh:
            json.dump(data, fh)

    @classmethod
    def load(cls, path, stopwords=None):
        import json
        with open(path, "r", encoding="utf-8") as fh:
            data = json.load(fh)
        idx = cls(
            k1=data.get("k1", DEFAULT_K1),
            b=data.get("b", DEFAULT_B),
            stopwords=stopwords,
            symbol_bonus=data.get("symbol_bonus", 2.0),
            docs_bonus=data.get("docs_bonus", 1.5),
            expansions=data.get("expansions", {}),
        )
        from models import Chunk
        for cd in data["chunks"]:
            idx.chunks.append(Chunk.from_dict(cd))
        idx.df = Counter(data["df"])
        idx.doc_lengths = list(data["doc_lengths"])
        # Rebuild tf from tokens.
        for chunk in idx.chunks:
            tokens = tokenize(chunk.text, idx.stopwords)
            for sym in chunk.symbols:
                for tok in tokenize(sym, None):
                    if tok not in tokens:
                        tokens.append(tok)
            for kw in chunk.path_keywords:
                if kw not in tokens:
                    tokens.append(kw)
            idx.doc_tokens.append(tokens)
            idx.tf.append(Counter(tokens))
            idx.N += 1
        idx._build_state = False
        idx._finalize()
        return idx
