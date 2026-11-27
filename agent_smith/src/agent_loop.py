"""The Thought -> Code -> Observation loop.

Specification implemented here (subject section V):

  1. Prompt       benchmark system prompt + the sandbox manual, always generated
                  from the connected server.
  2. Budget       history is compacted until the request fits the remaining input
                  budget (with a margin), checked BEFORE each request.
  3. Call         generation stops on ``<end_code>`` / ``</tool_call>``.
  4. Extraction   every accepted format; every liberty named back to the model.
  5. Execution    stdout/stderr/errors/timeouts become the next observation; a
                  submit warning is added to the *copy* of the history sent.
  6. Failures     key rotation on 429, model fallback with a handover note.
  7. Stop         final_answer, or a limit — always a SolutionOutput, never a
                  crash.
"""

from __future__ import annotations

import time
from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Optional, Sequence

from config import AgentConfig, SandboxConfig
from llm import (
    Completion,
    ProviderError,
    ProviderRegistry,
    TokenRotator,
    call_with_rotation,
    estimate_tokens,
)
from models import ExecutionResult, SolutionOutput, StepMetrics, Task
from parsing import extract_code

STOP_SEQUENCES = ("<end_code>", "</tool_call>")

MBPP_PROMPT = """You are Agent Smith, an autonomous coding agent solving MBPP tasks.

You get a problem description and a function signature. Write ONE Python function
that satisfies every hidden assertion, verify it with the `run_tests` tool, then
submit with `final_answer(complete_code)` where complete_code is the full,
self-contained source of your solution.

Rules:
- Reason first (Thought), then write exactly one Python code block (Code).
- The code block runs in a sandbox; print() output comes back as the Observation.
- Call run_tests(your_code, test_list=[...]) to check yourself before submitting.
- Never invent an observation: only the sandbox output is real.
- final_answer is always available and is not a tool.
"""

SWEBENCH_PROMPT = """You are Agent Smith, an autonomous coding agent fixing a real
GitHub issue in a checked-out repository.

Rules:
- Explore before editing: read_file, list_files, search_code,
  search_function_or_class_definition_in_code, find_references.
- Edit with edit_file (exact, unambiguous replacement), verify with run_command
  and run_tests, and produce the final diff with get_patch.
- Reason first (Thought), then write exactly one Python code block (Code).
- Never invent an observation: only real tool output is real.
- Submit with final_answer(summary) once the tests pass.
"""

PROMPTS = {"mbpp": MBPP_PROMPT, "swebench": SWEBENCH_PROMPT}


class AgentLoopError(RuntimeError):
    """A stop condition; converted into ``SolutionOutput(success=False)``."""


@dataclass
class Turn:
    """One conversation turn (assistant code or user observation)."""

    role: str
    content: str
    compactable: bool = False

    def to_message(self) -> Dict[str, str]:
        return {"role": self.role, "content": self.content}


class AgentLoop:
    """Drives the Thought -> Code -> Observation loop."""

    def __init__(self, sandbox: Any, registry: ProviderRegistry, model: str = "",
                 config: Optional[AgentConfig] = None,
                 system_prompt: Optional[str] = None,
                 sleep: Optional[Callable[[float], None]] = None) -> None:
        self.sandbox = sandbox
        self.registry = registry
        self.config = config or AgentConfig()
        self.model = model or self._default_model()
        self._explicit_prompt = system_prompt
        self._sleep = sleep or time.sleep
        self.turns: List[Turn] = []
        self.system_prompt = ""
        self.fallbacks: List[str] = []
        self.input_tokens_used: int = 0
        self.output_tokens_used: int = 0
        self.rotators: Dict[str, TokenRotator] = {}

    # -- setup -----------------------------------------------------------
    def _default_model(self) -> str:
        try:
            return self.registry.default_model()
        except ProviderError:
            return ""

    def build_system_prompt(self, benchmark: str) -> str:
        base = self._explicit_prompt or PROMPTS.get(benchmark, PROMPTS["mbpp"])
        manual = ""
        if hasattr(self.sandbox, "get_manual"):
            manual = self.sandbox.get_manual()
        return (base.strip() + "\n\n" + manual.strip() + "\n").strip() + "\n"

    # -- budget ----------------------------------------------------------
    @staticmethod
    def fit_view(system_prompt: str, turns: Sequence[Turn], task_prompt: str,
                 remaining_budget: int, keep_last: int = 4) -> List[Dict[str, str]]:
        """Send the task and the last turns intact; shorten older observations."""
        fixed = estimate_tokens(system_prompt) + estimate_tokens(task_prompt)
        messages: List[Dict[str, str]] = [{"role": "user", "content": task_prompt}]
        budget = max(0, remaining_budget - fixed)
        selected: List[Turn] = []
        used = 0
        for index, turn in enumerate(reversed(turns)):
            cost = estimate_tokens(turn.content)
            is_recent = index < keep_last
            if used + cost <= budget or is_recent:
                selected.append(turn)
                used += cost
            else:
                shortened = AgentLoop.truncate_history(turn.content,
                                                       max(64, budget - used))
                if shortened:
                    selected.append(Turn(turn.role, shortened, compactable=True))
                    used += estimate_tokens(shortened)
                break
        for turn in reversed(selected):
            messages.append(turn.to_message())
        return messages

    @staticmethod
    def truncate_history(text: str, max_chars: int = 400) -> str:
        """Keep the head and the tail of an old observation."""
        if len(text) <= max_chars:
            return text
        head = max(1, max_chars // 2)
        tail = max_chars - head
        omitted = len(text) - max_chars
        return text[:head] + f"\n...[older observation shortened: {omitted} chars]...\n" + text[-tail:]

    def _build_messages(self, task_prompt: str, extra_note: str = "") -> List[Dict[str, str]]:
        used = self.input_tokens_used
        remaining = max(0, self.config.max_input_tokens - used)
        margin = max(0, int(remaining * self.config.history_token_margin))
        view = self.fit_view(self.system_prompt, self.turns, task_prompt,
                             max(0, remaining - margin))
        if extra_note:
            if view and view[0]["role"] == "user":
                view[0] = {"role": "user", "content": extra_note + "\n\n" + view[0]["content"]}
            else:
                view.insert(0, {"role": "user", "content": extra_note})
        return [{"role": "system", "content": self.system_prompt}] + view

    # -- submission warning ----------------------------------------------
    def _submit_warning(self, iteration: int) -> str:
        remaining = self.config.max_iterations - iteration
        ratio_left = 1.0 - (self.input_tokens_used / max(1, self.config.max_input_tokens))
        if remaining <= self.config.submit_warning_iterations or \
                ratio_left <= self.config.submit_warning_ratio:
            return (
                "You are running out of budget "
                f"({max(0, remaining)} iteration(s) and {ratio_left:.0%} of the input budget left). "
                "Call final_answer(code) NOW with your complete solution."
            )
        return ""

    # -- main entry ------------------------------------------------------
    def run(self, task: Task) -> SolutionOutput:
        started = time.monotonic()
        self.system_prompt = self.build_system_prompt(task.benchmark)
        self.input_tokens_used = 0
        self.output_tokens_used = 0
        output = SolutionOutput(
            task_id=task.task_id,
            benchmark=task.benchmark,
            success=False,
            model=self.model,
            system_prompt=self.system_prompt,
        )
        self.rotators: Dict[str, TokenRotator] = {}
        last_code: Optional[str] = None
        handover = ""

        try:
            for iteration in range(1, self.config.max_iterations + 1):
                if time.monotonic() - started > self.config.max_wall_time_s:
                    raise AgentLoopError(
                        f"wall-time limit reached ({self.config.max_wall_time_s}s)"
                    )
                if self.input_tokens_used >= self.config.max_input_tokens:
                    raise AgentLoopError(
                        f"input-token limit reached ({self.config.max_input_tokens})"
                    )
                if self.output_tokens_used >= self.config.max_output_tokens:
                    raise AgentLoopError(
                        f"output-token limit reached ({self.config.max_output_tokens})"
                    )

                warning = self._submit_warning(iteration)
                messages = self._build_messages(task.prompt, extra_note=handover)
                if warning:
                    messages.append({"role": "user", "content": warning})

                completion, retries, model_used, url_used = self._call(
                    messages, output, iteration
                )
                self.input_tokens_used += completion.input_tokens
                self.output_tokens_used += completion.output_tokens

                extraction = extract_code(completion.text)
                step = StepMetrics(
                    step=iteration,
                    input_tokens=completion.input_tokens,
                    output_tokens=completion.output_tokens,
                    request_time_ms=completion.latency_ms,
                    llm_output=completion.text,
                    sandbox_input=extraction.code,
                    retries=retries,
                    model=model_used,
                    url=url_used,
                )

                self.turns.append(Turn("assistant", completion.text))

                if not extraction.found:
                    observation = extraction.observation(
                        "No Python code block was found in your answer. "
                        "Reply with exactly one fenced ```python block."
                    )
                    step.sandbox_output = observation
                    output.steps.append(step)
                    self.turns.append(Turn("user", observation, compactable=True))
                    continue

                last_code = extraction.code
                result = self._execute(extraction.code)
                observation = extraction.observation(result.observation())
                step.sandbox_output = observation
                output.steps.append(step)
                self.turns.append(Turn("user", observation, compactable=True))

                if result.final_answer is not None:
                    output.success = True
                    output.answer = result.final_answer
                    if task.benchmark == "mbpp":
                        output.code = _as_code(result.final_answer, last_code)
                    else:
                        patch = self._safe_patch()
                        output.patch = patch if patch else None
                    break
            else:
                raise AgentLoopError(
                    f"iteration limit reached ({self.config.max_iterations})"
                )
        except AgentLoopError as exc:
            output.success = False
            output.error = str(exc)
        except ProviderError as exc:
            output.success = False
            output.error = f"provider error: {exc}"
        except Exception as exc:  # noqa: BLE001 - never crash the caller
            output.success = False
            output.error = f"{type(exc).__name__}: {exc}"
        finally:
            try:
                self.sandbox.close()
            except Exception:  # noqa: BLE001
                pass

        output.total_input_tokens = sum(s.input_tokens for s in output.steps)
        output.total_output_tokens = sum(s.output_tokens for s in output.steps)
        output.total_time_ms = (time.monotonic() - started) * 1000
        if not output.success and output.code is None and last_code:
            output.code = last_code  # keep the last attempt for diagnostics
        return output

    # -- one LLM call with rotation + fallback ---------------------------
    def _call(self, messages: List[Dict[str, str]], output: SolutionOutput,
              iteration: int) -> tuple:
        candidates = [self.model] + [
            entry.model for entry in self.registry.ordered() if entry.model != self.model
        ]
        handover_note = ""
        last_error: Optional[ProviderError] = None
        for round_index in range(2):
            for candidate in candidates:
                try:
                    provider = self.registry.get(candidate)
                except ProviderError as exc:
                    last_error = exc
                    continue
                if candidate not in self.rotators:
                    from llm import make_rotator

                    self.rotators[candidate] = make_rotator(self.registry, candidate)
                payload = list(messages)
                if handover_note:
                    payload = payload + [{"role": "user", "content": handover_note}]
                try:
                    completion, retries = call_with_rotation(
                        provider, self.rotators[candidate], payload,
                        stop=STOP_SEQUENCES,
                        max_tokens=min(2048, self.config.max_output_tokens),
                    )
                except ProviderError as exc:
                    last_error = exc
                    if exc.status == 413:
                        # Shrink the history first before leaving this model.
                        messages = self._shrink(messages)
                        last_error = exc
                        continue
                    if exc.should_fallback or exc.retryable:
                        if candidate != self.model:
                            self.fallbacks.append(candidate)
                        handover_note = (
                            f"[handover] the previous model failed ({exc}); you are now "
                            f"{candidate}. The full history is above; continue and submit."
                        )
                        continue
                    raise
                if candidate != self.model:
                    self.model = candidate
                    output.model = candidate
                return completion, retries, completion.model, completion.url
            if round_index == 0 and self.config.retry_wait_s:
                self._sleep(self.config.retry_wait_s)
        raise AgentLoopError(f"every configured model failed: {last_error}")

    def _shrink(self, messages: List[Dict[str, str]]) -> List[Dict[str, str]]:
        """Shrink old observations after a 413."""
        shrunk: List[Dict[str, str]] = []
        for message in messages:
            if message["role"] == "user":
                shrunk.append({"role": "user",
                               "content": self.truncate_history(message["content"], 400)})
            else:
                shrunk.append(message)
        return shrunk

    # -- execution -------------------------------------------------------
    def _execute(self, code: str) -> ExecutionResult:
        try:
            return self.sandbox.execute(code)
        except Exception as exc:  # noqa: BLE001 - a dead sandbox is an observation
            return ExecutionResult(error=f"sandbox failure: {type(exc).__name__}: {exc}")

    def _safe_patch(self) -> str:
        tools = getattr(self.sandbox, "tools", None)
        if tools is None:
            return ""
        try:
            return tools.call("get_patch") or ""
        except Exception:  # noqa: BLE001
            return ""


def _as_code(answer: Any, fallback: str) -> str:
    """Use the submitted code when it is source, else the last executed block."""
    if isinstance(answer, str) and ("\n" in answer or answer.strip().startswith(("def ", "class ", "import ", "from "))):
        return answer
    return fallback


def build_loop(sandbox: Any, registry: ProviderRegistry, model: str = "",
               agent_config: Optional[AgentConfig] = None,
               system_prompt: Optional[str] = None) -> AgentLoop:
    return AgentLoop(sandbox, registry, model=model, config=agent_config,
                     system_prompt=system_prompt)
