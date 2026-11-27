"""Tests for the core data models."""

from __future__ import annotations

import pytest

from models import ExecutionResult, SolutionOutput, StepMetrics, Task, ToolCall


# -- Task ------------------------------------------------------------------
def test_task_minimal():
    task = Task(task_id="t1", benchmark="mbpp", prompt="do something")
    assert task.task_id == "t1"
    assert task.test_list == []
    assert task.to_dict()["benchmark"] == "mbpp"


def test_task_rejects_unknown_benchmark():
    with pytest.raises(ValueError, match="unknown benchmark"):
        Task(task_id="t1", benchmark="humaneval", prompt="x")


def test_task_rejects_empty_id():
    with pytest.raises(ValueError, match="task_id"):
        Task(task_id="", benchmark="mbpp", prompt="x")


def test_task_rejects_empty_prompt():
    with pytest.raises(ValueError, match="prompt"):
        Task(task_id="t1", benchmark="mbpp", prompt="   ")


def test_task_from_dict_keeps_extra_fields_as_metadata():
    task = Task.from_dict({
        "task_id": "t1", "benchmark": "swebench", "prompt": "fix",
        "repo_path": "/tmp/repo", "eval_script": "pytest", "difficulty": "hard",
    })
    assert task.repo_path == "/tmp/repo"
    assert task.eval_script == "pytest"
    assert task.metadata == {"difficulty": "hard"}


def test_task_round_trip():
    original = Task(task_id="t", benchmark="mbpp", prompt="p", test_list=["assert 1"])
    clone = Task.from_dict(original.to_dict())
    assert clone.to_dict() == original.to_dict()


# -- ExecutionResult -------------------------------------------------------
def test_execution_result_success_flag():
    assert ExecutionResult(stdout="ok").success
    assert not ExecutionResult(error="boom").success
    assert not ExecutionResult(timed_out=True).success


def test_observation_includes_every_stream():
    result = ExecutionResult(stdout="out\n", stderr="err\n", error="bad")
    text = result.observation()
    assert "out" in text
    assert "[stderr]" in text and "err" in text
    assert "[error]" in text and "bad" in text


def test_observation_signals_timeout_and_empty():
    assert "[timeout]" in ExecutionResult(timed_out=True).observation()
    assert ExecutionResult().observation() == "[no output]"


def test_observation_truncation_keeps_head_and_tail():
    result = ExecutionResult(stdout="A" * 500 + "B" * 500)
    text = result.observation(max_chars=100)
    assert "truncated" in text
    assert text.startswith("A")
    assert text.rstrip().endswith("B")


def test_execution_result_to_dict_is_json_shaped():
    result = ExecutionResult(stdout="x", tool_calls=[ToolCall("run_tests", {"code": "1"})])
    data = result.to_dict()
    assert data["stdout"] == "x"
    assert data["tool_calls"][0]["name"] == "run_tests"
    assert data["timed_out"] is False


# -- StepMetrics / SolutionOutput -----------------------------------------
def test_step_metrics_to_dict():
    step = StepMetrics(step=1, input_tokens=10, output_tokens=5, model="m")
    data = step.to_dict()
    assert data["step"] == 1
    assert data["input_tokens"] == 10
    assert data["model"] == "m"


def test_solution_output_round_trip():
    solution = SolutionOutput(
        task_id="t", benchmark="mbpp", success=True, answer="42",
        steps=[StepMetrics(step=1, input_tokens=3)], model="m",
        total_input_tokens=3, total_output_tokens=4,
    )
    data = solution.to_dict()
    clone = SolutionOutput.from_dict(data)
    assert clone.success is True
    assert clone.answer == "42"
    assert len(clone.steps) == 1
    assert clone.total_output_tokens == 4


def test_tool_call_defaults():
    call = ToolCall("read_file", {"filepath": "a.py"})
    assert call.status == "ok"
    assert call.error is None
