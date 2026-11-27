"""Structured JSON logging: one object per line.

Every record carries ``time``, ``level`` and ``msg`` plus whatever context the
caller passed (``type``, ``player``, ``ip``, ``command``, ``response``, ``code``),
which is what makes a session auditable after the fact.
"""

from __future__ import annotations

import json
import sys
import threading
from datetime import datetime, timezone
from typing import Any, Dict, IO, Optional


class JsonLogger:
    """A tiny thread-safe JSON-lines logger (stdlib only)."""

    def __init__(self, stream: Optional[IO[str]] = None, level: str = "INFO",
                 echo: bool = False) -> None:
        self.stream = stream if stream is not None else sys.stderr
        self.level = level.upper()
        self.echo = echo
        self.records: list = []
        self._lock = threading.Lock()
        self._levels = {"DEBUG": 10, "INFO": 20, "WARN": 30, "WARNING": 30, "ERROR": 40}

    def _enabled(self, level: str) -> bool:
        return self._levels.get(level.upper(), 20) >= self._levels.get(self.level, 20)

    def log(self, level: str, msg: str, **fields: Any) -> Dict[str, Any]:
        record: Dict[str, Any] = {
            "time": datetime.now(timezone.utc).astimezone().isoformat(),
            "level": level.upper(),
            "msg": msg,
        }
        record.update({key: value for key, value in fields.items() if value is not None})
        if self.echo:
            self.records.append(record)
        if not self._enabled(level):
            return record
        line = json.dumps(record, ensure_ascii=False, separators=(",", ":"))
        with self._lock:
            try:
                self.stream.write(line + "\n")
                self.stream.flush()
            except (ValueError, OSError):
                pass
        return record

    # -- convenience -----------------------------------------------------
    def info(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("INFO", msg, **fields)

    def warn(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("WARN", msg, **fields)

    def error(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("ERROR", msg, **fields)

    def debug(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("DEBUG", msg, **fields)


class NullLogger:
    """A logger that drops everything (used by tests and by the clients)."""

    def log(self, level: str, msg: str, **fields: Any) -> Dict[str, Any]:
        return {"level": level, "msg": msg, **fields}

    def info(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("INFO", msg, **fields)

    def warn(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("WARN", msg, **fields)

    def error(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("ERROR", msg, **fields)

    def debug(self, msg: str, **fields: Any) -> Dict[str, Any]:
        return self.log("DEBUG", msg, **fields)
