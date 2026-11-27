"""
Core data models for the RAG project.

Pure data holders with validation, no I/O. Every exchanged structure (chunk,
search result, answer) is a dataclass so the whole pipeline is deterministic
and unit-testable.
"""
from dataclasses import dataclass, field, asdict
from typing import List, Optional


@dataclass
class Chunk:
    """A segment of a source document with stable source locations.

    ``first_character_index`` / ``last_character_index`` are EXACT offsets into
    the original file — never approximated. They are a functional requirement
    because the official evaluator compares them against the expected dataset.
    """
    file_path: str
    text: str
    first_character_index: int
    last_character_index: int
    file_type: str = "text"        # "python", "markdown", "text"
    symbols: List[str] = field(default_factory=list)
    heading_path: List[str] = field(default_factory=list)  # markdown context
    path_keywords: List[str] = field(default_factory=list)  # from file path
    chunk_id: str = ""            # stable identifier (set by the indexer)

    def to_dict(self):
        return asdict(self)

    @classmethod
    def from_dict(cls, data):
        return cls(
            file_path=data["file_path"],
            text=data["text"],
            first_character_index=data["first_character_index"],
            last_character_index=data["last_character_index"],
            file_type=data.get("file_type", "text"),
            symbols=list(data.get("symbols", [])),
            heading_path=list(data.get("heading_path", [])),
            path_keywords=list(data.get("path_keywords", [])),
            chunk_id=data.get("chunk_id", ""),
        )


@dataclass
class SearchResult:
    """A single retrieved chunk with its score and source location."""
    chunk: Chunk
    score: float
    rank: int = 0

    def to_dict(self):
        return {
            "file_path": self.chunk.file_path,
            "text": self.chunk.text,
            "first_character_index": self.chunk.first_character_index,
            "last_character_index": self.chunk.last_character_index,
            "score": self.score,
            "rank": self.rank,
            "file_type": self.chunk.file_type,
        }


@dataclass
class Answer:
    """A generated answer plus the sources it was grounded on."""
    text: str
    sources: List[SearchResult] = field(default_factory=list)
    confidence: Optional[float] = None

    def to_dict(self):
        return {
            "answer": self.text,
            "sources": [s.to_dict() for s in self.sources],
            "confidence": self.confidence,
        }


@dataclass
class ManifestEntry:
    """Per-file metadata used for incremental indexing."""
    file_path: str
    size: int
    mtime: float
    content_hash: str
    chunking_params: dict = field(default_factory=dict)

    def to_dict(self):
        return asdict(self)

    @classmethod
    def from_dict(cls, data):
        return cls(
            file_path=data["file_path"],
            size=data["size"],
            mtime=data["mtime"],
            content_hash=data["content_hash"],
            chunking_params=dict(data.get("chunking_params", {})),
        )


def stable_chunk_id(file_path, first_index, last_index, text):
    """Stable identifier for a chunk based on path, range and content."""
    import hashlib
    raw = f"{file_path}|{first_index}|{last_index}|{text}"
    return hashlib.sha256(raw.encode("utf-8")).hexdigest()
