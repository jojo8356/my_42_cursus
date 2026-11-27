"""Tests for the restricted execution sandbox.

Covers: basic execution, persistence, timeouts, memory, output truncation,
final_answer, import allowlist, filesystem allowlist, network ban, dunder and
frame attribute rejection, builtins restriction, tool proxying and restart.
"""

from __future__ import annotations

import os

import pytest

from config import SandboxConfig
from mcp import ToolRegistry
from sandbox import Sandbox, SandboxError


# -- basic execution -------------------------------------------------------
def test_executes_and_captures_stdout(sandbox):
    result = sandbox.execute("print('hello')")
    assert result.stdout.strip() == "hello"
    assert result.error is None
    assert result.success


def test_captures_stderr(sandbox):
    import sys

    result = sandbox.execute("import sys; print('oops', file=sys.stderr)")
    assert "oops" in result.stderr


def test_reports_exception_as_error(sandbox):
    result = sandbox.execute("raise ValueError('boom')")
    assert "ValueError" in result.error
    assert "boom" in result.error
    assert not result.success


def test_reports_syntax_error(sandbox):
    result = sandbox.execute("def f(:\n    pass")
    assert "SyntaxError" in result.error


def test_empty_output(sandbox):
    result = sandbox.execute("x = 1")
    assert result.observation() == "[no output]"


def test_duration_is_measured(sandbox):
    result = sandbox.execute("x = 1")
    assert result.duration_s >= 0


# -- persistence -----------------------------------------------------------
def test_variables_persist_between_blocks(sandbox):
    sandbox.execute("counter = 10")
    result = sandbox.execute("print(counter + 5)")
    assert result.stdout.strip() == "15"


def test_functions_persist_between_blocks(sandbox):
    sandbox.execute("def double(x):\n    return 2 * x")
    result = sandbox.execute("print(double(21))")
    assert result.stdout.strip() == "42"


def test_persistence_can_be_disabled_by_history_clearing(sandbox):
    sandbox.execute("value = 1")
    assert "1" in sandbox.execute("print(value)").stdout


# -- final_answer ----------------------------------------------------------
def test_final_answer_stops_the_block(sandbox):
    result = sandbox.execute("final_answer(42)\nprint('never')")
    assert result.final_answer == 42
    assert "never" not in result.stdout


def test_final_answer_with_string(sandbox):
    assert sandbox.execute("final_answer('done')").final_answer == "done"


def test_final_answer_with_no_argument(sandbox):
    result = sandbox.execute("final_answer()")
    assert result.final_answer is None
    assert result.error is None


def test_final_answer_is_not_a_tool(sandbox):
    manual = sandbox.get_manual()
    assert "final_answer" in manual


# -- timeouts --------------------------------------------------------------
def test_infinite_loop_times_out(sandbox_config):
    box = Sandbox(config=SandboxConfig(
        allowed_directories=sandbox_config.allowed_directories, max_cpu_seconds=1
    ))
    try:
        result = box.execute("while True:\n    pass")
        assert result.timed_out
        assert not result.success
        assert "TimeoutError" in result.error
    finally:
        box.close()


def test_worker_is_replaced_after_timeout(sandbox_config):
    box = Sandbox(config=SandboxConfig(
        allowed_directories=sandbox_config.allowed_directories, max_cpu_seconds=1
    ))
    try:
        box.execute("while True:\n    pass")
        result = box.execute("print('alive')")
        assert result.stdout.strip() == "alive"
        assert result.error is None
    finally:
        box.close()


def test_history_is_replayed_after_timeout(sandbox_config):
    box = Sandbox(config=SandboxConfig(
        allowed_directories=sandbox_config.allowed_directories, max_cpu_seconds=1
    ))
    try:
        box.execute("kept = 7")
        box.execute("while True:\n    pass")
        result = box.execute("print(kept)")
        assert result.stdout.strip() == "7"
    finally:
        box.close()


def test_explicit_timeout_argument(sandbox):
    result = sandbox.execute("import time\ntime.sleep(0.1)", timeout=5)
    assert result.error is None and not result.timed_out


# -- output truncation -----------------------------------------------------
def test_long_output_is_truncated(sandbox_config):
    box = Sandbox(config=SandboxConfig(
        allowed_directories=sandbox_config.allowed_directories, max_output_chars=100
    ))
    try:
        result = box.execute("print('x' * 5000)")
        assert result.truncated
        assert len(result.stdout) <= 100
    finally:
        box.close()


# -- import allowlist ------------------------------------------------------
def test_allowed_import_works(sandbox):
    result = sandbox.execute("import math\nprint(int(math.sqrt(16)))")
    assert result.stdout.strip() == "4"


def test_forbidden_import_is_refused(sandbox):
    result = sandbox.execute("import subprocess")
    assert "ImportError" in result.error
    assert "allowlist" in result.error


def test_forbidden_import_os(sandbox_config):
    box = Sandbox(config=SandboxConfig(
        allowed_directories=sandbox_config.allowed_directories,
        authorized_imports=["math", "re"],
    ))
    try:
        assert "ImportError" in box.execute("import os").error
        assert box.execute("import math").error is None
    finally:
        box.close()


def test_relative_import_is_refused(sandbox):
    result = sandbox.execute("from . import thing")
    assert "ImportError" in result.error


def test_from_import_of_forbidden_module(sandbox):
    result = sandbox.execute("from subprocess import run")
    assert "ImportError" in result.error


# -- filesystem allowlist --------------------------------------------------
def test_write_inside_allowed_directory(workdir, sandbox):
    path = os.path.join(workdir, "note.txt")
    sandbox.execute(f"open({path!r}, 'w').write('hello')")
    result = sandbox.execute(f"print(open({path!r}).read())")
    assert result.stdout.strip() == "hello"


def test_read_outside_allowed_directory_is_refused(sandbox):
    result = sandbox.execute("open('/etc/passwd').read()")
    assert "PermissionError" in result.error


def test_traversal_out_of_allowed_directory_is_refused(workdir, sandbox):
    escape = os.path.join(workdir, "..", "..", "etc", "passwd")
    result = sandbox.execute(f"open({escape!r}).read()")
    assert "PermissionError" in result.error


def test_no_allowed_directory_denies_all_files(sandbox_config):
    box = Sandbox(config=SandboxConfig(max_cpu_seconds=2))
    try:
        result = box.execute("open('anything.txt', 'w').write('x')")
        assert "PermissionError" in result.error
    finally:
        box.close()


# -- network and subprocess ------------------------------------------------
def test_network_socket_is_blocked(sandbox):
    result = sandbox.execute("import socket\nsocket.socket()")
    assert result.error is not None


def test_network_module_not_importable(sandbox):
    result = sandbox.execute("import urllib.request")
    assert "ImportError" in result.error


def test_subprocess_module_not_importable(sandbox):
    assert "ImportError" in sandbox.execute("import subprocess").error


# -- builtins and attribute guards ----------------------------------------
@pytest.mark.parametrize("code,fragment", [
    ("eval('1+1')", "eval"),
    ("exec('x=1')", "exec"),
    ("compile('1', '<s>', 'eval')", "compile"),
    ("input()", "input"),
    ("breakpoint()", "breakpoint"),
    ("globals()", "globals"),
])
def test_forbidden_builtins_are_rejected(sandbox, code, fragment):
    result = sandbox.execute(code)
    assert result.error is not None
    assert fragment in result.error


def test_dunder_attribute_is_rejected_at_parse_time(sandbox):
    result = sandbox.execute("().__class__.__bases__[0].__subclasses__()")
    assert "SecurityError" in result.error


def test_frame_attribute_gi_frame_is_rejected(sandbox):
    result = sandbox.execute("def g():\n    yield 1\nprint(g().gi_frame)")
    assert "SecurityError" in result.error


def test_frame_attribute_f_back_is_rejected(sandbox):
    result = sandbox.execute("import sys\nsys._getframe().f_back")
    assert result.error is not None


def test_classic_subclass_escape_is_blocked(sandbox):
    code = (
        "[c for c in ().__class__.__base__.__subclasses__() "
        "if c.__name__ == 'Popen']"
    )
    assert "SecurityError" in sandbox.execute(code).error


def test_system_exit_cannot_kill_the_agent(sandbox):
    result = sandbox.execute("raise SystemExit(0)")
    assert "SystemExit" in result.error
    assert sandbox.execute("print('still here')").stdout.strip() == "still here"


def test_keyboard_interrupt_is_contained(sandbox):
    result = sandbox.execute("raise KeyboardInterrupt")
    assert "KeyboardInterrupt" in result.error
    assert sandbox.execute("print('alive')").stdout.strip() == "alive"


# -- tools -----------------------------------------------------------------
def test_tool_proxy_calls_the_registry(sandbox_config):
    registry = ToolRegistry("test")
    calls = []

    def echo(value):
        """Echo a value."""
        calls.append(value)
        return value

    registry.register(echo)
    box = Sandbox(config=sandbox_config, tools=registry)
    try:
        result = box.execute("print(echo('hi'))")
        assert result.stdout.strip() == "hi"
        assert calls == ["hi"]
        assert result.tool_calls[0].name == "echo"
    finally:
        box.close()


def test_tool_failure_is_reported_to_the_model(sandbox_config):
    registry = ToolRegistry("test")

    def boom():
        """Always fails."""
        raise RuntimeError("tool exploded")

    registry.register(boom)
    box = Sandbox(config=sandbox_config, tools=registry)
    try:
        result = box.execute("boom()")
        assert "tool exploded" in result.error
    finally:
        box.close()


def test_unknown_tool_is_not_exposed(sandbox):
    result = sandbox.execute("not_a_tool()")
    assert "NameError" in result.error


def test_manual_lists_tools_and_limits(sandbox_config):
    registry = ToolRegistry("test")

    def ping():
        """Ping the server."""
        return "pong"

    registry.register(ping)
    box = Sandbox(config=sandbox_config, tools=registry)
    try:
        manual = box.get_manual()
        assert "ping()" in manual
        assert "Memory limit" in manual
        assert "Network: forbidden" in manual
        assert "final_answer" in manual
    finally:
        box.close()


def test_manual_without_tools(sandbox):
    manual = sandbox.get_manual()
    assert "No MCP server is connected" in manual


# -- lifecycle -------------------------------------------------------------
def test_execute_after_close_raises(sandbox):
    sandbox.close()
    with pytest.raises(SandboxError):
        sandbox.execute("x = 1")


def test_close_is_idempotent(sandbox):
    sandbox.close()
    sandbox.close()


def test_context_manager(workdir):
    with Sandbox(config=SandboxConfig(allowed_directories=[workdir])) as box:
        assert box.execute("print(1)").stdout.strip() == "1"


def test_history_is_exposed(sandbox):
    sandbox.execute("a = 1")
    sandbox.execute("b = 2")
    assert len(sandbox.history) == 2


def test_failed_block_is_not_recorded_in_history(sandbox):
    sandbox.execute("a = 1")
    sandbox.execute("raise ValueError('x')")
    assert len(sandbox.history) == 1


# -- memory ----------------------------------------------------------------
def test_memory_limit_is_applied(sandbox_config):
    box = Sandbox(config=SandboxConfig(
        allowed_directories=sandbox_config.allowed_directories,
        max_memory_mb=64, max_cpu_seconds=5,
    ))
    try:
        result = box.execute("payload = bytearray(400 * 1024 * 1024)\nprint(len(payload))")
        assert result.error is not None or result.timed_out
    finally:
        box.close()
