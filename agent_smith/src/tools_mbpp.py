"""MBPP tool server.

One mandatory tool — ``run_tests(code, test_list=None)`` — plus ``check_syntax``
as an extra so the agent can catch a ``SyntaxError`` before spending an iteration
on a failed test run.

``run_tests`` runs the candidate against each assertion, one after the other, in a
separate process under a wall-clock timeout (SIGTERM on expiry), and returns
``{"success": bool, "output": str}`` where ``output`` holds the first failing
assertion plus captured stdout/stderr, truncated with an explicit notice.
"""

from __future__ import annotations

import io
import multiprocessing as mp
import os
import signal
import sys
import traceback
from contextlib import redirect_stderr, redirect_stdout
from typing import Any, Dict, List, Optional

from mcp import ToolRegistry

DEFAULT_MAX_OUTPUT = 4000
DEFAULT_TIMEOUT = 10.0


def _truncate(text: str, limit: int) -> str:
    if len(text) <= limit:
        return text
    keep_tail = limit // 2
    keep_head = limit - keep_tail
    return (
        text[:keep_head]
        + f"\n...[output truncated: {len(text) - limit} chars omitted]...\n"
        + text[-keep_tail:]
    )


def _child_run(code: str, tests: List[str], conn) -> None:
    """Run the candidate once per assertion, reporting the first failure."""
    buffer_out, buffer_err = io.StringIO(), io.StringIO()
    namespace: Dict[str, Any] = {"__name__": "__mbpp__"}
    try:
        with redirect_stdout(buffer_out), redirect_stderr(buffer_err):
            exec(compile(code, "<candidate>", "exec"), namespace)  # noqa: S102
    except BaseException as exc:  # noqa: BLE001 - reported to the model
        conn.send({
            "success": False,
            "output": "the candidate code raised while being defined:\n"
                      + "".join(traceback.format_exception_only(type(exc), exc)),
            "stdout": buffer_out.getvalue(),
            "stderr": buffer_err.getvalue(),
            "failing": None,
        })
        return

    for index, assertion in enumerate(tests, start=1):
        try:
            with redirect_stdout(buffer_out), redirect_stderr(buffer_err):
                exec(compile(assertion, f"<test {index}>", "exec"), namespace)  # noqa: S102
        except AssertionError:
            conn.send({
                "success": False,
                "output": f"assertion {index} failed: {assertion.strip()}",
                "stdout": buffer_out.getvalue(),
                "stderr": buffer_err.getvalue(),
                "failing": assertion,
            })
            return
        except BaseException as exc:  # noqa: BLE001
            conn.send({
                "success": False,
                "output": f"assertion {index} raised: {assertion.strip()}\n"
                          + "".join(traceback.format_exception_only(type(exc), exc)),
                "stdout": buffer_out.getvalue(),
                "stderr": buffer_err.getvalue(),
                "failing": assertion,
            })
            return

    conn.send({
        "success": True,
        "output": f"all {len(tests)} assertions passed",
        "stdout": buffer_out.getvalue(),
        "stderr": buffer_err.getvalue(),
        "failing": None,
    })


class MBPPTools:
    """The MBPP tool server."""

    def __init__(self, test_list: Optional[List[str]] = None,
                 timeout: float = DEFAULT_TIMEOUT,
                 max_output: int = DEFAULT_MAX_OUTPUT) -> None:
        self.test_list = list(test_list or [])
        self.timeout = timeout
        self.max_output = max_output

    # -- mandatory tool --------------------------------------------------
    def run_tests(self, code: str, test_list: Optional[List[str]] = None) -> Dict[str, Any]:
        """Run ``code`` against every assertion and report the first failure."""
        tests = list(test_list) if test_list is not None else list(self.test_list)
        if not tests:
            return {
                "success": False,
                "output": "no test list was provided: pass test_list=[...] or attach tests to the task",
            }
        try:
            compile(code, "<candidate>", "exec")
        except SyntaxError as exc:
            return {
                "success": False,
                "output": f"SyntaxError: {exc.msg} (line {exc.lineno})",
            }

        context = mp.get_context("fork") if "fork" in mp.get_all_start_methods() else mp.get_context()
        parent_conn, child_conn = context.Pipe()
        process = context.Process(target=_child_run, args=(code, tests, child_conn), daemon=True)
        process.start()
        child_conn.close()

        process.join(self.timeout)
        timed_out = process.is_alive()
        payload: Dict[str, Any]
        if timed_out:
            # SIGTERM first (the subject's choice), then kill if it survives.
            try:
                if hasattr(process, "pid") and process.pid:
                    os.kill(process.pid, signal.SIGTERM)
            except (OSError, ValueError, AttributeError):
                pass
            process.join(0.5)
            if process.is_alive():
                process.kill()
                process.join(0.5)
            payload = {
                "success": False,
                "output": f"TimeoutError: the tests did not finish within {self.timeout:g}s",
                "stdout": "",
                "stderr": "",
                "failing": None,
            }
        else:
            try:
                payload = parent_conn.recv() if parent_conn.poll(0.5) else {
                    "success": False,
                    "output": "the test process died without reporting a result",
                    "stdout": "",
                    "stderr": "",
                    "failing": None,
                }
            except (EOFError, OSError):
                payload = {
                    "success": False,
                    "output": "the test process died without reporting a result",
                    "stdout": "",
                    "stderr": "",
                    "failing": None,
                }
        parent_conn.close()

        output = payload.get("output", "")
        stdout = _truncate(payload.get("stdout", ""), self.max_output)
        stderr = _truncate(payload.get("stderr", ""), self.max_output)
        if stdout.strip():
            output += "\n[stdout]\n" + stdout.rstrip("\n")
        if stderr.strip():
            output += "\n[stderr]\n" + stderr.rstrip("\n")
        return {"success": bool(payload.get("success")), "output": _truncate(output, self.max_output * 2)}

    # -- extra tool ------------------------------------------------------
    def check_syntax(self, code: str) -> Dict[str, Any]:
        """Check that ``code`` parses, without running it."""
        try:
            compile(code, "<candidate>", "exec")
        except SyntaxError as exc:
            return {
                "success": False,
                "output": f"SyntaxError: {exc.msg} (line {exc.lineno}, offset {exc.offset})",
            }
        return {"success": True, "output": "the code parses"}

    # -- registry --------------------------------------------------------
    def registry(self, name: str = "mbpp") -> ToolRegistry:
        registry = ToolRegistry(name)
        registry.register(self.run_tests, name="run_tests")
        registry.register(self.check_syntax, name="check_syntax")
        return registry
