"""Tests for the benchmark evaluator and the trace writer."""

from __future__ import annotations

import json
import os

import pytest

from evaluator import BenchmarkReport, TaskOutcome, evaluate, load_tasks
from models import SolutionOutput, StepMetrics, Task
from reporting import load_solution, write_report, write_solution


def _task(task_id, benchmark="mbpp"):
    return Task(task_id=task_id, benchmark=benchmark, prompt="solve")


def _solution(task_id, success, **kwargs):
    return SolutionOutput(task_id=task_id, benchmark="mbpp", success=success, **kwargs)


# -- evaluate --------------------------------------------------------------
def test_evaluate_counts_passes():
    tasks = [_task("1"), _task("2"), _task("3")]
    results = {"1": True, "2": True, "3": False}
    report = evaluate(tasks, lambda task: _solution(task.task_id, results[task.task_id]))
    assert report.total == 3
    assert report.passed == 2
    assert report.failed == 1
    assert report.pass_at_1 == pytest.approx(2 / 3)


def test_evaluate_empty_is_zero():
    report = evaluate([], lambda task: _solution(task.task_id, True))
    assert report.total == 0
    assert report.pass_at_1 == 0.0


def test_evaluate_captures_the_error():
    report = evaluate([_task("1")], lambda task: _solution("1", False, error="boom"))
    assert report.failures()[0].error == "boom"


def test_evaluate_survives_a_runner_crash():
    def runner(task):
        raise RuntimeError("runner exploded")

    report = evaluate([_task("1")], runner)
    assert report.passed == 0
    assert "runner exploded" in report.outcomes[0].error


def test_evaluate_sums_tokens_and_time():
    def runner(task):
        return _solution("1", True, steps=[StepMetrics(step=1, input_tokens=10, output_tokens=5)],
                         total_input_tokens=10, total_output_tokens=5, total_time_ms=42.0)

    report = evaluate([_task("1")], runner)
    assert report.total_input_tokens == 10
    assert report.total_output_tokens == 5
    assert report.total_time_ms == 42.0


def test_evaluate_records_the_step_count():
    def runner(task):
        return _solution("1", True, steps=[StepMetrics(step=i) for i in range(1, 4)])

    assert evaluate([_task("1")], runner).outcomes[0].steps == 3


def test_evaluate_infers_the_benchmark():
    report = evaluate([_task("1", "swebench")], lambda task: _solution("1", True))
    assert report.benchmark == "swebench"


def test_evaluate_explicit_benchmark_wins():
    report = evaluate([_task("1")], lambda task: _solution("1", True), benchmark="custom")
    assert report.benchmark == "custom"


# -- report ----------------------------------------------------------------
def test_report_to_dict_shape():
    report = BenchmarkReport("mbpp", [TaskOutcome("1", True), TaskOutcome("2", False, "e")])
    data = report.to_dict()
    assert data["total"] == 2
    assert data["pass_at_1"] == 0.5
    assert len(data["outcomes"]) == 2


def test_report_summary_mentions_failures():
    report = BenchmarkReport("mbpp", [TaskOutcome("bad_task", False, "kaboom")])
    summary = report.summary()
    assert "bad_task" in summary
    assert "kaboom" in summary
    assert "pass@1" in summary


def test_outcome_to_dict():
    data = TaskOutcome("1", True, steps=2).to_dict()
    assert data["success"] is True
    assert data["steps"] == 2


# -- load_tasks ------------------------------------------------------------
def test_load_tasks_from_a_list():
    tasks = load_tasks([{"task_id": "1", "benchmark": "mbpp", "prompt": "p"}])
    assert len(tasks) == 1
    assert tasks[0].task_id == "1"


def test_load_tasks_from_a_wrapped_list():
    tasks = load_tasks({"tasks": [{"task_id": "1", "benchmark": "mbpp", "prompt": "p"}]})
    assert len(tasks) == 1


def test_load_single_task_object():
    tasks = load_tasks({"task_id": "1", "benchmark": "mbpp", "prompt": "p"})
    assert len(tasks) == 1


def test_load_tasks_rejects_garbage():
    with pytest.raises(ValueError):
        load_tasks("not a task")


# -- reporting -------------------------------------------------------------
def test_write_solution_is_atomic_and_readable(tmp_path):
    path = tmp_path / "solution.json"
    solution = _solution("1", True, answer=42)
    written = write_solution(solution, str(path))
    assert os.path.exists(written)
    data = json.loads(path.read_text())
    assert data["answer"] == 42
    assert not [p for p in os.listdir(tmp_path) if p.endswith(".tmp")]


def test_write_solution_accepts_a_dict(tmp_path):
    path = tmp_path / "d.json"
    write_solution({"a": 1}, str(path))
    assert json.loads(path.read_text()) == {"a": 1}


def test_load_solution_round_trip(tmp_path):
    path = tmp_path / "solution.json"
    write_solution(_solution("1", True, answer="x",
                             steps=[StepMetrics(step=1, input_tokens=2)]), str(path))
    restored = load_solution(str(path))
    assert restored.answer == "x"
    assert len(restored.steps) == 1


def test_write_report(tmp_path):
    report = BenchmarkReport("mbpp", [TaskOutcome("1", True)])
    path = tmp_path / "report.json"
    write_report(report, str(path))
    assert json.loads(path.read_text())["passed"] == 1


def test_write_solution_creates_the_directory(tmp_path):
    path = tmp_path / "nested" / "dir" / "solution.json"
    write_solution(_solution("1", True), str(path))
    assert path.exists()
