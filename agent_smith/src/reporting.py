"""Writing the run trace (``solution.json``) atomically."""

from __future__ import annotations

import json
import os
import tempfile
from typing import Any, Dict, Union

from models import SolutionOutput


def write_solution(output: Union[SolutionOutput, Dict[str, Any]], path: str) -> str:
    """Write the trace atomically and return the absolute path."""
    data = output.to_dict() if isinstance(output, SolutionOutput) else output
    directory = os.path.dirname(os.path.abspath(path)) or "."
    os.makedirs(directory, exist_ok=True)
    handle, tmp_path = tempfile.mkstemp(dir=directory, suffix=".tmp")
    try:
        with os.fdopen(handle, "w", encoding="utf-8") as fh:
            json.dump(data, fh, indent=2, ensure_ascii=False, default=str)
        os.replace(tmp_path, path)
    except BaseException:
        if os.path.exists(tmp_path):
            os.unlink(tmp_path)
        raise
    return os.path.abspath(path)


def load_solution(path: str) -> SolutionOutput:
    with open(path, encoding="utf-8") as fh:
        return SolutionOutput.from_dict(json.load(fh))


def write_report(report: Any, path: str) -> str:
    data = report.to_dict() if hasattr(report, "to_dict") else report
    return write_solution(data, path)
