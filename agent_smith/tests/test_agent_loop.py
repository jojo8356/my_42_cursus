"""Tests for the Thought -> Code -> Observation loop."""

from __future__ import annotations

import pytest

from agent_loop import (
    AgentLoop,
    AgentLoopError,
    Turn,
    build_loop,
)
from config import AgentConfig, SandboxConfig
from llm import ProviderError, ProviderRegistry, ScriptedProvider
from models import Task
from sandbox import Sandbox
from tools_mbpp import MBPPTools

GOOD_CODE = "def add(a, b):\n    return a + b"


# -- fixtures --------------------------------------------------------------
@pytest.fixture
def make_loop(workdir):
    """Build a loop around a scripted provider and the MBPP tools."""

    def factory(script, test_list=None, agent_config=None, model="scripted",
                repeat_last=False):
        registry = ProviderRegistry([])
        provider = ScriptedProvider(script, model=model, repeat_last=repeat_last)
        registry.register(model, provider)
        tools = MBPPTools(test_list=test_list or ["assert add(1, 2) == 3"], timeout=5)
        sandbox = Sandbox(
            config=SandboxConfig(allowed_directories=[workdir], max_cpu_seconds=5),
            tools=tools.registry(),
        )
        loop = AgentLoop(
            sandbox, registry, model=model,
            config=agent_config or AgentConfig(max_iterations=5),
        )
        return loop, provider, sandbox

    return factory


@pytest.fixture
def task():
    return Task(task_id="t1", benchmark="mbpp",
                prompt="Write add(a, b).", entry_point="add",
                test_list=["assert add(1, 2) == 3"])


# -- success paths ---------------------------------------------------------
def test_successful_run_submits_and_exposes_the_code(make_loop, task):
    script = [
        f"Thought: write it.\n```python\n{GOOD_CODE}\nprint(run_tests('''{GOOD_CODE}'''))\n```",
        f"Thought: done.\n```python\nfinal_answer('''{GOOD_CODE}''')\n```",
    ]
    loop, _, _ = make_loop(script)
    solution = loop.run(task)
    assert solution.success is True
    assert solution.error is None
    assert len(solution.steps) == 2
    assert GOOD_CODE.strip() in (solution.code or "")


def test_steps_record_the_trace(make_loop, task):
    script = [
        f"```python\n{GOOD_CODE}\n```",
        f"```python\nfinal_answer('''{GOOD_CODE}''')\n```",
    ]
    loop, _, _ = make_loop(script)
    solution = loop.run(task)
    first = solution.steps[0]
    assert first.step == 1
    assert "Thought" not in first.sandbox_input
    assert first.llm_output.startswith("```python")
    assert first.sandbox_output is not None
    assert first.input_tokens > 0
    assert first.model == "scripted"


def test_totals_match_the_steps(make_loop, task):
    script = ["```python\nx = 1\n```", "```python\nfinal_answer(1)\n```"]
    loop, _, _ = make_loop(script)
    solution = loop.run(task)
    assert solution.total_input_tokens == sum(s.input_tokens for s in solution.steps)
    assert solution.total_output_tokens == sum(s.output_tokens for s in solution.steps)
    assert solution.total_time_ms >= 0


def test_system_prompt_contains_the_manual(make_loop, task):
    loop, _, _ = make_loop(["```python\nfinal_answer(1)\n```"])
    solution = loop.run(task)
    assert "Sandbox manual" in solution.system_prompt
    assert "run_tests" in solution.system_prompt
    assert "final_answer" in solution.system_prompt


def test_observation_is_fed_back_to_the_model(make_loop, task):
    script = [
        "```python\nprint('MARKER_42')\n```",
        "```python\nfinal_answer(1)\n```",
    ]
    loop, provider, _ = make_loop(script)
    loop.run(task)
    second_call = provider.calls[1]
    joined = "\n".join(message["content"] for message in second_call)
    assert "MARKER_42" in joined


def test_tool_observation_reaches_the_model(make_loop, task):
    script = [
        f"```python\nprint(run_tests('''{GOOD_CODE}'''))\n```",
        "```python\nfinal_answer(1)\n```",
    ]
    loop, provider, _ = make_loop(script)
    solution = loop.run(task)
    assert "all 1 assertions passed" in solution.steps[0].sandbox_output
    assert "assertions passed" in "\n".join(m["content"] for m in provider.calls[1])


# -- error handling --------------------------------------------------------
def test_no_code_block_is_reported_back(make_loop, task):
    script = ["I will think about it.", "```python\nfinal_answer(1)\n```"]
    loop, provider, _ = make_loop(script)
    solution = loop.run(task)
    assert solution.success is True
    assert "No Python code block" in solution.steps[0].sandbox_output
    assert "No Python code block" in "\n".join(m["content"] for m in provider.calls[1])


def test_code_error_is_an_observation_not_a_crash(make_loop, task):
    script = ["```python\nraise ValueError('boom')\n```", "```python\nfinal_answer(1)\n```"]
    loop, _, _ = make_loop(script)
    solution = loop.run(task)
    assert solution.success is True
    assert "ValueError" in solution.steps[0].sandbox_output


def test_iteration_limit_returns_a_failed_solution(make_loop, task):
    loop, _, _ = make_loop(
        ["```python\nx = 1\n```"] * 10,
        agent_config=AgentConfig(max_iterations=3),
    )
    solution = loop.run(task)
    assert solution.success is False
    assert "iteration limit" in solution.error
    assert len(solution.steps) == 3


def test_output_token_limit(make_loop, task):
    loop, _, _ = make_loop(
        ["```python\nx = 1\n```"] * 10,
        agent_config=AgentConfig(max_iterations=10, max_output_tokens=5),
    )
    solution = loop.run(task)
    assert solution.success is False
    assert "output-token limit" in solution.error


def test_input_token_limit(make_loop, task):
    loop, _, _ = make_loop(
        ["```python\nx = 1\n```"] * 10,
        agent_config=AgentConfig(max_iterations=10, max_input_tokens=10),
    )
    solution = loop.run(task)
    assert solution.success is False
    assert "input-token limit" in solution.error


def test_wall_time_limit(make_loop, task):
    loop, _, _ = make_loop(
        ["```python\nx = 1\n```"] * 50,
        agent_config=AgentConfig(max_iterations=50, max_wall_time_s=0.001),
    )
    solution = loop.run(task)
    assert solution.success is False
    assert "wall-time limit" in solution.error


def test_provider_exhaustion_is_reported(make_loop, task):
    loop, _, _ = make_loop(["```python\nx = 1\n```"])
    solution = loop.run(task)
    assert solution.success is False
    assert "provider" in (solution.error or "").lower() or "failed" in (solution.error or "").lower()


def test_last_code_is_kept_on_failure(make_loop, task):
    loop, _, _ = make_loop(
        ["```python\npartial = 41\n```"],
        agent_config=AgentConfig(max_iterations=1),
    )
    solution = loop.run(task)
    assert solution.success is False
    assert solution.code is not None
    assert "partial = 41" in solution.code


def test_sandbox_is_closed_after_the_run(make_loop, task):
    loop, _, sandbox = make_loop(["```python\nfinal_answer(1)\n```"])
    loop.run(task)
    assert sandbox._closed is True


# -- notes -----------------------------------------------------------------
def test_extraction_notes_reach_the_model(make_loop, task):
    script = [
        "```bash\necho hi\n```",
        "```python\nfinal_answer(1)\n```",
    ]
    loop, provider, _ = make_loop(script)
    solution = loop.run(task)
    assert "[note]" in solution.steps[0].sandbox_output
    assert "[note]" in "\n".join(m["content"] for m in provider.calls[1])


def test_several_blocks_note(make_loop, task):
    script = [
        "```python\na = 1\n```\nprose\n```python\nb = 2\n```",
        "```python\nfinal_answer(1)\n```",
    ]
    loop, _, _ = make_loop(script)
    solution = loop.run(task)
    assert "2 code blocks" in solution.steps[0].sandbox_output


# -- budget compaction -----------------------------------------------------
def test_fit_view_keeps_the_task_and_last_turns():
    turns = [Turn("user", "old observation " * 200, compactable=True)] * 6
    messages = AgentLoop.fit_view("system", turns, "TASK", remaining_budget=100_000)
    assert messages[0]["content"] == "TASK"
    assert len(messages) > 1


def test_fit_view_truncates_when_the_budget_is_tight():
    turns = [Turn("user", "x" * 5000, compactable=True) for _ in range(4)]
    messages = AgentLoop.fit_view("sys", turns, "TASK", remaining_budget=400, keep_last=1)
    joined = "\n".join(m["content"] for m in messages)
    assert "shortened" in joined or len(joined) < 20_000


def test_truncate_history_keeps_head_and_tail():
    text = "H" * 1000 + "T" * 1000
    result = AgentLoop.truncate_history(text, 100)
    assert result.startswith("H")
    assert result.endswith("T")
    assert "shortened" in result


def test_truncate_history_leaves_short_text_alone():
    assert AgentLoop.truncate_history("short", 100) == "short"


# -- submit warning --------------------------------------------------------
def test_submit_warning_fires_near_the_end(workdir):
    loop = AgentLoop(
        Sandbox(config=SandboxConfig(allowed_directories=[workdir])),
        ProviderRegistry([]),
        config=AgentConfig(max_iterations=3, submit_warning_iterations=2),
    )
    warning = loop._submit_warning(iteration=1)
    assert "final_answer" in warning


def test_no_submit_warning_early(workdir):
    loop = AgentLoop(
        Sandbox(config=SandboxConfig(allowed_directories=[workdir])),
        ProviderRegistry([]),
        config=AgentConfig(max_iterations=10, submit_warning_iterations=1),
    )
    assert loop._submit_warning(iteration=1) == ""


def test_submit_warning_is_not_stored_in_the_history(make_loop, task):
    script = ["```python\nx = 1\n```"] * 3 + ["```python\nfinal_answer(1)\n```"]
    loop, _, _ = make_loop(
        script, agent_config=AgentConfig(max_iterations=4, submit_warning_iterations=3)
    )
    loop.run(task)
    stored = "\n".join(turn.content for turn in loop.turns)
    assert "running out of budget" not in stored


# -- fallback --------------------------------------------------------------
def test_fallback_to_the_next_model(workdir, task):
    failing = ScriptedProvider([{"raise": "model down", "status": 503}], model="weak")
    backup = ScriptedProvider(["```python\nfinal_answer('backup')\n```"], model="strong")
    registry = ProviderRegistry([
        __import__("llm").ModelEntry(model="weak", provider="p", url="u", strength=0),
        __import__("llm").ModelEntry(model="strong", provider="p", url="u", strength=0),
    ])
    registry.register("weak", failing)
    registry.register("strong", backup)
    tools = MBPPTools(test_list=["assert True"])
    sandbox = Sandbox(config=SandboxConfig(allowed_directories=[workdir]), tools=tools.registry())
    loop = AgentLoop(sandbox, registry, model="weak", config=AgentConfig(max_iterations=3))
    solution = loop.run(task)
    assert solution.success is True
    assert solution.answer == "backup"
    assert solution.model == "strong"


def test_handover_note_is_sent_to_the_backup(workdir, task):
    failing = ScriptedProvider([{"raise": "down", "status": 500}], model="weak")
    backup = ScriptedProvider(["```python\nfinal_answer(1)\n```"], model="strong")
    registry = ProviderRegistry([
        __import__("llm").ModelEntry(model="weak", provider="p", url="u"),
        __import__("llm").ModelEntry(model="strong", provider="p", url="u"),
    ])
    registry.register("weak", failing)
    registry.register("strong", backup)
    sandbox = Sandbox(config=SandboxConfig(allowed_directories=[workdir]))
    loop = AgentLoop(sandbox, registry, model="weak", config=AgentConfig(max_iterations=2))
    loop.run(task)
    joined = "\n".join(m["content"] for m in backup.calls[0])
    assert "handover" in joined


def test_all_models_failing_is_a_failed_solution(workdir, task):
    registry = ProviderRegistry([])
    registry.register("solo", ScriptedProvider([{"raise": "down", "status": 500}], model="solo"))
    sandbox = Sandbox(config=SandboxConfig(allowed_directories=[workdir]))
    loop = AgentLoop(sandbox, registry, model="solo", config=AgentConfig(max_iterations=2))
    solution = loop.run(task)
    assert solution.success is False
    assert solution.error


def test_build_loop_helper(workdir):
    registry = ProviderRegistry([])
    registry.register("m", ScriptedProvider(["x"], model="m"))
    sandbox = Sandbox(config=SandboxConfig(allowed_directories=[workdir]))
    loop = build_loop(sandbox, registry, model="m")
    assert isinstance(loop, AgentLoop)


def test_agent_loop_error_is_a_runtime_error():
    assert issubclass(AgentLoopError, RuntimeError)


# -- prompts ---------------------------------------------------------------
def test_swebench_prompt_is_selected(workdir):
    loop = AgentLoop(Sandbox(config=SandboxConfig(allowed_directories=[workdir])),
                     ProviderRegistry([]))
    prompt = loop.build_system_prompt("swebench")
    assert "GitHub issue" in prompt


def test_explicit_system_prompt_wins(workdir):
    loop = AgentLoop(Sandbox(config=SandboxConfig(allowed_directories=[workdir])),
                     ProviderRegistry([]), system_prompt="CUSTOM PROMPT")
    assert "CUSTOM PROMPT" in loop.build_system_prompt("mbpp")
