"""Persistent high-score table for Pacman.

Scores are stored in a JSON file. Names are validated (up to 10 alphanumeric
characters or spaces) and scores must be non-negative integers. Only the top
10 entries are kept, sorted from highest to lowest.
"""
from __future__ import annotations

import json
import os
import re
from os import PathLike
from typing import TypedDict

NAME_MAX = 10
NAME_RE = re.compile(r"[A-Za-z0-9 ]{1,10}")


class HighscoreRecord(TypedDict):
    """A validated name and score pair."""

    name: str
    score: int


def validate_name(name: object) -> str:
    """Return a normalized name or raise ``ValueError`` if invalid."""
    if not isinstance(name, str):
        raise ValueError("name must be a string")
    normalized = name.strip()
    if NAME_RE.fullmatch(normalized) is None:
        raise ValueError("name must be 1-10 alphanumeric characters or spaces")
    return normalized


def validate_score(score: object) -> int:
    """Return a non-negative integer score or raise ``ValueError``."""
    if type(score) is not int:
        raise ValueError("score must be an integer")
    if score < 0:
        raise ValueError("score must be non-negative")
    return score


class HighscoreStore:
    """Load, validate and persist the top-10 high scores."""

    TOP_N = 10

    def __init__(self, path: str | PathLike[str]) -> None:
        self.path = path
        self.records: list[HighscoreRecord] = []

    def load(self) -> list[HighscoreRecord]:
        """Load records from JSON (empty if missing or invalid)."""
        self.records = []
        try:
            with open(self.path, "r", encoding="utf-8") as score_file:
                data = json.load(score_file)
        except (OSError, ValueError):
            return self.records
        if not isinstance(data, list):
            return self.records
        for entry in data:
            if not isinstance(entry, dict):
                continue
            if "name" not in entry or "score" not in entry:
                continue
            try:
                record: HighscoreRecord = {
                    "name": validate_name(entry["name"]),
                    "score": validate_score(entry["score"]),
                }
            except ValueError:
                continue
            self.records.append(record)
        self.records.sort(key=lambda record: record["score"], reverse=True)
        self.records = self.records[: self.TOP_N]
        return self.records

    def save(self) -> None:
        """Write the current records to the JSON file."""
        parent = os.path.dirname(os.path.abspath(self.path))
        os.makedirs(parent or ".", exist_ok=True)
        with open(self.path, "w", encoding="utf-8") as score_file:
            json.dump(self.records, score_file, indent=2, ensure_ascii=False)

    def add(self, name: object, score: object) -> HighscoreRecord:
        """Validate and add a score, keeping only the top 10."""
        record: HighscoreRecord = {
            "name": validate_name(name),
            "score": validate_score(score),
        }
        self.records.append(record)
        self.records.sort(key=lambda item: item["score"], reverse=True)
        self.records = self.records[: self.TOP_N]
        return record

    def top(self) -> list[HighscoreRecord]:
        """Return a shallow copy of the current top-10 records."""
        return list(self.records)

    def is_new_record(self, score: object) -> bool:
        """Whether ``score`` would make the top 10."""
        validated_score = validate_score(score)
        if len(self.records) < self.TOP_N:
            return True
        return validated_score > self.records[-1]["score"]
