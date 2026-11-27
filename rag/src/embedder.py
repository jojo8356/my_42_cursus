"""
Embedding models for semantic retrieval.

The project keeps every processing step local. The default embedder is a
deterministic **hashing bag-of-words** embedder that requires no model
download and no network access — ideal for fast, reproducible tests. It maps
tokens into a fixed-dimension vector using a stable hash, then L2-normalises
the result so that inner-product search is equivalent to cosine similarity.

An ``OpenAIEmbedder`` is provided for real semantic quality when the ``openai``
package and an API key are available; it is imported lazily so the core never
depends on it.
"""
import hashlib
import math
import re

from bm25 import tokenize

_TOKEN_RE = re.compile(r"[A-Za-z0-9_]+")


class BaseEmbedder:
    """Interface for any embedding model."""

    def embed_text(self, text):
        raise NotImplementedError

    def embed_documents(self, texts):
        return [self.embed_text(t) for t in texts]

    @property
    def dimensions(self):
        raise NotImplementedError


class HashingEmbedder(BaseEmbedder):
    """Deterministic hashing bag-of-words embedder.

    Each token is hashed to a signed bucket in a fixed-dimension vector; the
    vector is then L2-normalised so inner product == cosine similarity.
    """

    def __init__(self, dimensions=256):
        self._dimensions = dimensions

    @property
    def dimensions(self):
        return self._dimensions

    def _hash_token(self, token):
        raw = hashlib.sha256(token.encode("utf-8")).digest()
        # Use the first 8 bytes as an unsigned int, then derive a signed
        # bucket and a sign (+1 / -1) for signed hashing (reduces collisions).
        value = int.from_bytes(raw[:8], "big")
        bucket = value % self._dimensions
        sign = 1 if (value >> 63) & 1 else -1
        return bucket, sign

    def embed_text(self, text):
        vec = [0.0] * self._dimensions
        tokens = tokenize(text, None)
        for tok in tokens:
            bucket, sign = self._hash_token(tok)
            vec[bucket] += sign
        # L2 normalisation.
        norm = math.sqrt(sum(v * v for v in vec))
        if norm > 0:
            vec = [v / norm for v in vec]
        return vec


class OpenAIEmbedder(BaseEmbedder):
    """Embedder backed by the OpenAI embeddings API (optional).

    Imported lazily so the core project does not depend on ``openai``.
    """

    def __init__(self, model="text-embedding-3-small", api_key=None):
        import os
        self.model = model
        self.api_key = api_key or os.environ.get("OPENAI_API_KEY")
        if not self.api_key:
            raise ValueError("OPENAI_API_KEY is required for OpenAIEmbedder")

    @property
    def dimensions(self):
        # text-embedding-3-small defaults to 1536 dimensions.
        return 1536

    def embed_text(self, text):
        return self.embed_documents([text])[0]

    def embed_documents(self, texts):
        import openai
        client = openai.OpenAI(api_key=self.api_key)
        resp = client.embeddings.create(model=self.model, input=texts)
        return [list(item.embedding) for item in resp.data]


def make_embedder(kind="hashing", **kwargs):
    """Factory: build an embedder by name ('hashing' or 'openai')."""
    if kind == "hashing":
        return HashingEmbedder(**kwargs)
    if kind == "openai":
        return OpenAIEmbedder(**kwargs)
    raise ValueError(f"unknown embedder kind: {kind}")
