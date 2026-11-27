"""
Persistent configuration for the RAG project (``config.ini``).

All configurable parameters are available here and restored between sessions.
Values are validated with sane fallbacks so a corrupted file never crashes the
application.
"""
import configparser
import os

DEFAULTS = {
    "repo_path": ".",
    "work_dir": "data",
    "embedder": "hashing",
    "generator": "template",
    "chunk_max_size": "2000",
    "chunk_overlap": "200",
    "top_k": "5",
    "hybrid": "false",
    "retrieval_mode": "bm25",
}


class RAGConfig:
    """Typed view over a config.ini file with validation and fallbacks."""

    def __init__(self, **overrides):
        self._data = dict(DEFAULTS)
        for key, value in overrides.items():
            if value is not None:
                self._data[key] = value

    # ------------------------------------------------------------------ access
    @property
    def repo_path(self):
        return self._data["repo_path"]

    @property
    def work_dir(self):
        return self._data["work_dir"]

    @property
    def embedder(self):
        return self._data["embedder"]

    @property
    def generator(self):
        return self._data["generator"]

    @property
    def chunk_max_size(self):
        return self._as_int("chunk_max_size", 2000, 100, 100000)

    @property
    def chunk_overlap(self):
        return self._as_int("chunk_overlap", 200, 0, 10000)

    @property
    def top_k(self):
        return self._as_int("top_k", 5, 1, 1000)

    @property
    def hybrid(self):
        return self._data.get("hybrid", "false").lower() in ("1", "true", "yes")

    @property
    def retrieval_mode(self):
        return self._data.get("retrieval_mode", "bm25")

    def _as_int(self, key, default, low, high):
        try:
            v = int(self._data.get(key, default))
        except (TypeError, ValueError):
            return default
        if v < low or v > high:
            return default
        return v

    # ------------------------------------------------------------------ io
    @classmethod
    def load(cls, path):
        """Load config from an ini file, falling back to defaults."""
        cfg = cls()
        if not os.path.exists(path):
            return cfg
        parser = configparser.ConfigParser()
        parser.read(path, encoding="utf-8")
        if parser.has_section("rag"):
            for key, value in parser.items("rag"):
                cfg._data[key] = value
        return cfg

    def save(self, path):
        parser = configparser.ConfigParser()
        parser["rag"] = self._data
        os.makedirs(os.path.dirname(os.path.abspath(path)) or ".",
                    exist_ok=True)
        with open(path, "w", encoding="utf-8") as fh:
            parser.write(fh)

    def to_dict(self):
        return dict(self._data)
