"""Core data models for Agent Smith.

Pure data containers: no I/O, no side effects. Every model can be turned into a
JSON-serialisable dict so a run can be fully traced in ``solution.json``.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional


# ---------------------------------------------------------------------------
# Tasks
# ---------------------------------------------------------------------------
@dataclass
class Task:
    """A benchmark task (one MBPP problem or one SWE-bench issue)."""

    task_id: str
    benchmark: str  # "mbpp" | "swebench"
    prompt: str
    entry_point: Optional[str] = None
    test_list: List[str] = field(default_factory=list)
    eval_script: Optional[str] = None
    repo_path: Optional[str] = None
    docker_image: Optional[str] = None
    metadata: Dict[str, Any] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if self.benchmark not in ("mbpp", "swebench"):
            raise ValueError(f"unknown benchmark: {self.benchmark!r}")
        if not self.task_id:
            raise ValueError("task_id must not be empty")
        if not isinstance(self.prompt, str) or not self.prompt.strip():
            raise ValueError("prompt must not be empty")

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "Task":
        known = {
            "task_id", "benchmark", "prompt", "entry_point", "test_list",
            "eval_script", "repo_path", "docker_image", "metadata",
        }
        kwargs = {k: v for k, v in data.items() if k in known}
        extra = {k: v for k, v in data.items() if k not in known}
        if extra:
            kwargs.setdefault("metadata", {})
            merged = dict(kwargs.get("metadata") or {})
            merged.update(extra)
            kwargs["metadata"] = merged
        return cls(**kwargs)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "task_id": self.task_id,
            "benchmark": self.benchmark,
            "prompt": self.prompt,
            "entry_point": self.entry_point,
            "test_list": list(self.test_list),
            "eval_script": self.eval_script,
            "repo_path": self.repo_path,
            "docker_image": self.docker_image,
            "metadata": dict(self.metadata),
        
        }


# ---------------------------------------------------------------------------
# Execution
# ---------------------------------------------------------------------------
@dataclass
class ToolCall:
    """One tool invocation made from inside the sandbox."""

    name: str
    arguments: Dict[str, Any] = field(default_factory=dict)
    status: str = "ok"
    error: Optional[str] = None

    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "arguments": self.arguments,
            "status": self.status,
            "error": self.error,
        }


@dataclass
class ExecutionResult:
    """Outcome of one ``Sandbox.execute()`` call."""

    stdout: str = ""
    stderr: str = ""
    error: Optional[str] = None
    timed_out: bool = False
    final_answer: Any = None
    duration_s: float = 0.0
    tool_calls: List[ToolCall] = field(default_factory=list)
    truncated: bool = False
    exit_code: Optional[int] = None

    @property
    def success(self) -> bool:
        return self.error is None and not self.timed_out

    def observation(self, max_chars: Optional[int] = None) -> str:
        """Render the result as the text handed back to the model."""
        parts: List[str] = []
        if self.stdout:
            parts.append(self.stdout if self.stdout.endswith("\n") else self.stdout + "\n")
        if self.stderr:
            parts.append("[stderr]\n" + self.stderr.rstrip("\n") + "\n")
        if self.error:
            parts.append("[error]\n" + self.error.rstrip("\n") + "\n")
        if self.timed_out:
            parts.append("[timeout] execution exceeded the sandbox time limit\n")
        if self.truncated:
            parts.append("[truncated] output exceeded the maximum length and was cut\n")
        if not parts:
            parts.append("[no output]")
        text = "".join(parts)
        if max_chars is not None and len(text) > max_chars:
            keep_tail = max_chars // 2
            keep_head = max_chars - keep_tail
            text = (
                text[:keep_head]
                + f"\n...[truncated {len(text) - max_chars} chars]...\n"
                + text[-keep_tail:]
            )
        return text

    def to_dict(self) -> Dict[str, Any]:
        return {
            "stdout": self.stdout,
            "stderr": self.stderr,
            "error": self.error,
            "timed_out": self.timed_out,
            "final_answer": self.final_answer,
            "duration_s": self.duration_s,
            "tool_calls": [c.to_dict() for c in self.tool_calls],
            "truncated": self.truncated,
            "exit_code": self.exit_code,
        }


# ---------------------------------------------------------------------------
# Tracing
# ---------------------------------------------------------------------------
@dataclass
class StepMetrics:
    """Metrics for a single agent step: one LLM generate -> sandbox execute cycle."""

    step: int
    input_tokens: int = 0
    output_tokens: int = 0
    request_time_ms: float = 0.0
    llm_output: str = ""
    sandbox_input: str = ""
    sandbox_output: str = ""
    retries: int = 0
    model: str = ""
    url: str = ""

    def to_dict(self) -> Dict[str, Any]:
        return {
            "step": self.step,
            "input_tokens": self.input_tokens,
            "output_tokens": self.output_tokens,
            "request_time_ms": self.request_time_ms,
            "llm_output": self.llm_output,
            "sandbox_input": self.sandbox_input,
            "sandbox_output": self.sandbox_output,
            "retries": self.retries,
            "model": self.model,
            "url": self.url,
        }


@dataclass
class SolutionOutput:
    """The full trace of one run, written to ``solution.json``."""

    task_id: str
    benchmark: str
    success: bool
    answer: Any = None
    error: Optional[str] = None
    steps: List[StepMetrics] = field(default_factory=list)
    model: str = ""
    system_prompt: str = ""
    total_input_tokens: int = 0
    total_output_tokens: int = 0
    total_time_ms: float = 0.0
    patch: Optional[str] = None
    code: Optional[str] = None

    def to_dict(self) -> Dict[str, Any]:
        return {
            "task_id": self.task_id,
            "benchmark": self.benchmark,
            "success": self.success,
            "answer": self.answer,
            "error": self.error,
            "steps": [s.to_dict() for s in self.steps],
            "model": self.model,
            "system_prompt": self.system_prompt,
            "total_input_tokens": self.total_input_tokens,
            "total_output_tokens": self.total_output_tokens,
            "total_time_ms": self.total_time_ms,
            "patch": self.patch,
            "code": self.code,
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "SolutionOutput":
        steps = [StepMetrics(**s) for s in data.get("steps", [])]
        return cls(
            task_id=data["task_id"],
            benchmark=data["benchmark"],
            success=bool(data.get("success")),
            answer=data.get("answer"),
            error=data.get("error"),
            steps=steps,
            model=data.get("model", ""),
            system_prompt=data.get("system_prompt", ""),
            total_input_tokens=data.get("total_input_tokens", 0),
            total_output_tokens=data.get("total_output_tokens", 0),
            total_time_ms=data.get("total_time_ms", 0.0),
            patch=data.get("patch"),
            code=data.get("code"),
        )
