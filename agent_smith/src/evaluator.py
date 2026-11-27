"""Benchmark evaluation (MBPP and SWE-bench).

Given a list of tasks and a runner (``task -> SolutionOutput``), produce a
deterministic report: pass@1, per-task diagnostics, token and time accounting.
The runner is injectable so the evaluator is testable without any LLM.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Callable, Dict, Iterable, List, Optional

from models import SolutionOutput, Task


@dataclass
class TaskOutcome:
    """One evaluated task."""

    task_id: str
    success: bool
    error: Optional[str] = None
    steps: int = 0
    input_tokens: int = 0
    output_tokens: int = 0
    time_ms: float = 0.0
    answer: Any = None

    def to_dict(self) -> Dict[str, Any]:
        return {
            "task_id": self.task_id,
            "success": self.success,
            "error": self.error,
            "steps": self.steps,
            "input_tokens": self.input_tokens,
            "output_tokens": self.output_tokens,
            "time_ms": self.time_ms,
            "answer": self.answer,
        }


@dataclass
class BenchmarkReport:
    """Aggregated result of one benchmark run."""

    benchmark: str
    outcomes: List[TaskOutcome] = field(default_factory=list)

    # -- aggregates ------------------------------------------------------
    @property
    def total(self) -> int:
        return len(self.outcomes)

    @property
    def passed(self) -> int:
        return sum(1 for outcome in self.outcomes if outcome.success)

    @property
    def failed(self) -> int:
        return self.total - self.passed

    @property
    def pass_at_1(self) -> float:
        return self.passed / self.total if self.total else 0.0

    @property
    def total_input_tokens(self) -> int:
        return sum(outcome.input_tokens for outcome in self.outcomes)

    @property
    def total_output_tokens(self) -> int:
        return sum(outcome.output_tokens for outcome in self.outcomes)

    @property
    def total_time_ms(self) -> float:
        return sum(outcome.time_ms for outcome in self.outcomes)

    def failures(self) -> List[TaskOutcome]:
        return [outcome for outcome in self.outcomes if not outcome.success]

    def to_dict(self) -> Dict[str, Any]:
        return {
            "benchmark": self.benchmark,
            "total": self.total,
            "passed": self.passed,
            "failed": self.failed,
            "pass_at_1": round(self.pass_at_1, 4),
            "total_input_tokens": self.total_input_tokens,
            "total_output_tokens": self.total_output_tokens,
            "total_time_ms": round(self.total_time_ms, 2),
            "outcomes": [outcome.to_dict() for outcome in self.outcomes],
        }

    def summary(self) -> str:
        lines = [
            f"benchmark: {self.benchmark}",
            f"passed:    {self.passed}/{self.total} (pass@1 = {self.pass_at_1:.1%})",
            f"tokens:    {self.total_input_tokens} in / {self.total_output_tokens} out",
            f"time:      {self.total_time_ms / 1000:.1f}s",
        ]
        for outcome in self.failures():
            lines.append(f"  FAILED {outcome.task_id}: {outcome.error or 'unknown error'}")
        return "\n".join(lines)


Runner = Callable[[Task], SolutionOutput]


def evaluate(tasks: Iterable[Task], runner: Runner,
             benchmark: Optional[str] = None) -> BenchmarkReport:
    """Run ``runner`` on every task and aggregate the outcomes."""
    task_list = list(tasks)
    name = benchmark or (task_list[0].benchmark if task_list else "unknown")
    outcomes: List[TaskOutcome] = []
    for task in task_list:
        try:
            solution = runner(task)
        except Exception as exc:  # noqa: BLE001 - a crash is a failed task
            outcomes.append(TaskOutcome(
                task_id=task.task_id, success=False,
                error=f"{type(exc).__name__}: {exc}",
            ))
            continue
        outcomes.append(TaskOutcome(
            task_id=task.task_id,
            success=bool(solution.success),
            error=solution.error,
            steps=len(solution.steps),
            input_tokens=solution.total_input_tokens,
            output_tokens=solution.total_output_tokens,
            time_ms=solution.total_time_ms,
            answer=solution.answer,
        ))
    return BenchmarkReport(benchmark=name, outcomes=outcomes)


def load_tasks(data: Any) -> List[Task]:
    """Accept a single task dict or a ``tasks`` list."""
    if isinstance(data, dict):
        if "tasks" in data:
            data = data["tasks"]
        else:
            return [Task.from_dict(data)]
    if not isinstance(data, list):
        raise ValueError("a tasks file must contain a task object or a `tasks` list")
    return [Task.from_dict(item) for item in data]
