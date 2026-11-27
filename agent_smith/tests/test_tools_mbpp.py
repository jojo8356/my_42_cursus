"""Tests for the MBPP tool server (run_tests, check_syntax)."""

from __future__ import annotations

import pytest

from tools_mbpp import MBPPTools

GOOD = "def add(a, b):\n    return a + b\n"
BAD = "def add(a, b):\n    return a - b\n"
TESTS = ["assert add(1, 2) == 3", "assert add(-1, 1) == 0"]


@pytest.fixture
def server():
    return MBPPTools(test_list=TESTS, timeout=5)


# -- run_tests -------------------------------------------------------------
def test_passing_solution(server):
    result = server.run_tests(GOOD)
    assert result["success"] is True
    assert "passed" in result["output"]


def test_failing_solution_reports_first_failure(server):
    result = server.run_tests(BAD)
    assert result["success"] is False
    assert "assertion 1 failed" in result["output"]
    assert "add(1, 2) == 3" in result["output"]


def test_explicit_test_list_overrides_the_default(server):
    result = server.run_tests("def f():\n    return 1", test_list=["assert f() == 1"])
    assert result["success"] is True


def test_missing_test_list_is_reported():
    server = MBPPTools()
    result = server.run_tests("def f():\n    return 1")
    assert result["success"] is False
    assert "no test list" in result["output"]


def test_syntax_error_is_reported_before_running(server):
    result = server.run_tests("def add(a, b)\n    return a + b")
    assert result["success"] is False
    assert "SyntaxError" in result["output"]


def test_exception_during_definition(server):
    result = server.run_tests("raise RuntimeError('nope')")
    assert result["success"] is False
    assert "RuntimeError" in result["output"]


def test_exception_inside_an_assertion(server):
    result = server.run_tests("def add(a, b):\n    return a + b", test_list=["assert add('a', 1) == 0"])
    assert result["success"] is False
    assert "assertion 1 raised" in result["output"]
    assert "TypeError" in result["output"]


def test_stdout_is_captured(server):
    code = (
        "def add(a, b):\n"
        "    print('debug', a, b)\n"
        "    return a + b\n"
    )
    result = server.run_tests(code)
    assert result["success"] is True
    assert "debug" in result["output"]


def test_stderr_is_captured(server):
    code = (
        "import sys\n"
        "def add(a, b):\n"
        "    print('warn', file=sys.stderr)\n"
        "    return a + b\n"
    )
    result = server.run_tests(code)
    assert "[stderr]" in result["output"]


def test_timeout_is_enforced():
    server = MBPPTools(test_list=["assert True"], timeout=1)
    code = "while True:\n    pass\n"
    result = server.run_tests(code)
    assert result["success"] is False
    assert "Timeout" in result["output"] or "did not finish" in result["output"]


def test_long_output_is_truncated():
    server = MBPPTools(test_list=["assert True"], timeout=5, max_output=200)
    code = "print('y' * 5000)\n"
    result = server.run_tests(code)
    assert "truncated" in result["output"]


def test_assertion_with_custom_message(server):
    result = server.run_tests(BAD, test_list=['assert add(1, 2) == 3, "sum is wrong"'])
    assert result["success"] is False


def test_solution_using_an_import(server):
    code = (
        "import math\n"
        "def add(a, b):\n"
        "    return math.floor(a + b)\n"
    )
    assert server.run_tests(code)["success"] is True


# -- check_syntax ----------------------------------------------------------
def test_check_syntax_ok(server):
    result = server.check_syntax(GOOD)
    assert result["success"] is True


def test_check_syntax_reports_position(server):
    result = server.check_syntax("def f(:\n    pass")
    assert result["success"] is False
    assert "SyntaxError" in result["output"]
    assert "line" in result["output"]


# -- registry --------------------------------------------------------------
def test_registry_exposes_both_tools(server):
    registry = server.registry()
    assert registry.names() == ["check_syntax", "run_tests"]
    assert "run_tests" in registry.manual()
    assert "code: str" in registry.manual()


def test_registry_call_through_the_manual_contract(server):
    registry = server.registry()
    assert registry.call("check_syntax", code=GOOD)["success"] is True
    assert registry.call("run_tests", code=GOOD)["success"] is True
