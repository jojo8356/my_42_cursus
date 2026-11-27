"""
Persistent high-score table for Pacman.

Scores are stored in a JSON file. Names are validated (up to 10 alphanumeric
characters or spaces) and scores must be non-negative integers. Only the top
10 entries are kept, sorted from highest to lowest.
"""
import json
import os
import re

NAME_MAX = 10
NAME_RE = re.compile(r"^[A-Za-z0-9 ]{1,10}$")


def validate_name(name):
    """Return a normalized name or raise ``ValueError`` if invalid."""
    if not isinstance(name, str):
        raise ValueError("name must be a string")
    name = name.strip()
    if not NAME_RE.match(name):
        raise ValueError("name must be 1-10 alphanumeric characters or spaces")
    return name


def validate_score(score):
    """Return an int score or raise ``ValueError`` if invalid."""
    try:
        s = int(score)
    except (TypeError, ValueError):
        raise ValueError("score must be an integer")
    if s < 0:
        raise ValueError("score must be non-negative")
    return s


class HighscoreStore:
    """Load, validate and persist the top-10 high scores."""

    TOP_N = 10

    def __init__(self, path):
        self.path = path
        self.records = []

    def load(self):
        """Load records from the JSON file (empty if missing/invalid)."""
        self.records = []
        try:
            with open(self.path, "r", encoding="utf-8") as fh:
                data = json.load(fh)
        except (OSError, ValueError):
            return self.records
        if not isinstance(data, list):
            return self.records
        for entry in data:
            if isinstance(entry, dict) and "name" in entry and "score" in entry:
                try:
                    name = validate_name(entry["name"])
                    score = validate_score(entry["score"])
                    self.records.append({"name": name, "score": score})
                except ValueError:
                    continue
        self.records.sort(key=lambda r: r["score"], reverse=True)
        return self.records

    def save(self):
        """Write the current records to the JSON file."""
        os.makedirs(os.path.dirname(os.path.abspath(self.path)) or ".", exist_ok=True)
        with open(self.path, "w", encoding="utf-8") as fh:
            json.dump(self.records, fh, indent=2, ensure_ascii=False)

    def add(self, name, score):
        """Validate and add a score, keeping only the top 10."""
        name = validate_name(name)
        score = validate_score(score)
        record = {"name": name, "score": score}
        self.records.append(record)
        self.records.sort(key=lambda r: r["score"], reverse=True)
        self.records = self.records[: self.TOP_N]
        return record

    def top(self):
        """Return the current top-10 records (sorted)."""
        return list(self.records)

    def is_new_record(self, score):
        """Whether ``score`` would make the top-10."""
        if len(self.records) < self.TOP_N:
            return True
        return score > self.records[-1]["score"]
