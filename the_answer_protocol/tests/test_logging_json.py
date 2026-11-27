from __future__ import annotations

import io
import json

from logging_json import JsonLogger, NullLogger


def test_json_logger_writes_one_compact_unicode_record_per_line():
    stream = io.StringIO()
    logger = JsonLogger(stream=stream)
    record = logger.info("player connected", type="session", player="Chani",
                         ip="127.0.0.1", absent=None)
    encoded = stream.getvalue().splitlines()
    assert len(encoded) == 1
    decoded = json.loads(encoded[0])
    assert decoded == record
    assert decoded["level"] == "INFO"
    assert decoded["msg"] == "player connected"
    assert decoded["player"] == "Chani"
    assert "absent" not in decoded
    assert ": " not in encoded[0]


def test_json_logger_level_filter_echo_and_convenience_methods():
    stream = io.StringIO()
    logger = JsonLogger(stream=stream, level="ERROR", echo=True)
    logger.debug("debug")
    logger.info("info")
    logger.warn("warning")
    logger.error("error")
    assert len(logger.records) == 4
    assert len(stream.getvalue().splitlines()) == 1
    assert json.loads(stream.getvalue())["msg"] == "error"
    assert logger.log("WARNING", "warning alias")["level"] == "WARNING"
    assert logger._enabled("ERROR")
    assert not logger._enabled("DEBUG")


def test_json_logger_handles_closed_or_broken_streams_gracefully():
    class BadStream:
        def write(self, _line):
            raise OSError("disk full")
        def flush(self):
            raise ValueError("closed")
    logger = JsonLogger(stream=BadStream())
    assert logger.info("still safe")["msg"] == "still safe"


def test_null_logger_convenience_calls_return_structured_dicts():
    logger = NullLogger()
    assert logger.info("hi", player="Paul") == {
        "level": "INFO", "msg": "hi", "player": "Paul"
    }
    assert logger.warn("warn")["level"] == "WARN"
    assert logger.error("error")["level"] == "ERROR"
    assert logger.debug("debug")["level"] == "DEBUG"
    assert logger.log("CUSTOM", "message", x=1) == {
        "level": "CUSTOM", "msg": "message", "x": 1
    }
