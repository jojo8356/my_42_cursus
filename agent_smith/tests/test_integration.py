"""End-to-end tests: the full pipeline, the CLI and the trace file."""

from __future__ import annotations

import json
import os
import subprocess
import sys

import pytest

from agent_loop import AgentLoop
from config import AgentConfig, SandboxConfig
from evaluator import evaluate
from llm import ProviderRegistry, ScriptedProvider
from reporting import load_solution, write_solution
from sandbox import Sandbox
from tools_mbpp import MBPPTools
from tools_swebench import SWEBenchTools
from models import Task

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CLI = os.path.join(ROOT, "agent_smith.py")

GOOD = "def add(a, b):\n    return a + b"


# -- full MBPP pipeline ----------------------------------------------------
def test_full_mbpp_pipeline(workdir, mbpp_task, tmp_path):
    script = [
        f"Thought: write it.\n```python\n{GOOD}\nprint(run_tests('''{GOOD}'''))\n```",
        f"Thought: verified.\n```python\nfinal_answer('''{GOOD}''')\n```",
    ]
    registry = ProviderRegistry([])
    registry.register("s", ScriptedProvider(script, model="s"))
    sandbox = Sandbox(
        config=SandboxConfig(allowed_directories=[workdir], max_cpu_seconds=5),
        tools=MBPPTools(test_list=mbpp_task.test_list).registry(),
    )
    loop = AgentLoop(sandbox, registry, model="s", config=AgentConfig(max_iterations=4))
    solution = loop.run(mbpp_task)

    assert solution.success
    assert solution.answer
    assert "return a + b" in solution.code

    path = write_solution(solution, str(tmp_path / "solution.json"))
    restored = load_solution(path)
    assert restored.success
    assert len(restored.steps) == 2
    assert restored.system_prompt
    assert restored.total_input_tokens > 0


def test_trace_contains_sandbox_io(workdir, mbpp_task, tmp_path):
    script = [f"```python\n{GOOD}\nprint('SENTINEL')\n```", "```python\nfinal_answer(1)\n```"]
    registry = ProviderRegistry([])
    registry.register("s", ScriptedProvider(script, model="s"))
    sandbox = Sandbox(config=SandboxConfig(allowed_directories=[workdir]))
    loop = AgentLoop(sandbox, registry, model="s", config=AgentConfig(max_iterations=3))
    write_solution(loop.run(mbpp_task), str(tmp_path / "s.json"))
    data = json.loads((tmp_path / "s.json").read_text())
    assert "SENTINEL" in data["steps"][0]["sandbox_output"]
    assert data["steps"][0]["llm_output"]
    assert data["steps"][0]["sandbox_input"]


# -- full SWE-bench pipeline ----------------------------------------------
def test_full_swebench_pipeline(git_repo, tmp_path):
    task = Task(
        task_id="swe_1", benchmark="swebench",
        prompt="Fix add().", repo_path=str(git_repo),
        eval_script="python3 test_calc.py",
    )
    script = [
        "```python\nprint(read_file('calc.py', 1, 3))\n```",
        "```python\nprint(edit_file('calc.py', 'return a - b  # bug', 'return a + b'))\n"
        "print(run_tests())\n```",
        "```python\nfinal_answer('fixed')\n```",
    ]
    registry = ProviderRegistry([])
    registry.register("s", ScriptedProvider(script, model="s"))
    tools = SWEBenchTools(root=str(git_repo), eval_script=task.eval_script)
    sandbox = Sandbox(config=SandboxConfig(allowed_directories=[str(git_repo)],
                                           max_cpu_seconds=10), tools=tools.registry())
    loop = AgentLoop(sandbox, registry, model="s", config=AgentConfig(max_iterations=5))
    solution = loop.run(task)

    assert solution.success
    assert solution.answer == "fixed"
    assert solution.patch is not None
    assert "+    return a + b" in solution.patch
    assert (git_repo / "calc.py").read_text().startswith("def add(a, b):\n    return a + b")


def test_swebench_without_submission_still_reports_the_patch(git_repo):
    task = Task(task_id="swe_2", benchmark="swebench", prompt="fix",
                repo_path=str(git_repo), eval_script="true")
    script = ["```python\nprint(edit_file('calc.py', 'return a - b  # bug', 'return a + b'))\n```"] * 3
    registry = ProviderRegistry([])
    registry.register("s", ScriptedProvider(script, model="s"))
    tools = SWEBenchTools(root=str(git_repo), eval_script="true")
    sandbox = Sandbox(config=SandboxConfig(allowed_directories=[str(git_repo)]),
                      tools=tools.registry())
    loop = AgentLoop(sandbox, registry, model="s", config=AgentConfig(max_iterations=2))
    solution = loop.run(task)
    assert solution.success is False
    assert "iteration limit" in solution.error


# -- benchmark evaluation --------------------------------------------------
def test_evaluation_over_several_tasks(workdir):
    tasks = [
        Task(task_id=f"t{i}", benchmark="mbpp", prompt=f"p{i}",
             test_list=[f"assert f{i}() == {i}"])
        for i in range(1, 4)
    ]

    def runner(task):
        number = task.task_id[-1]
        code = f"def f{number}():\n    return {number}"
        script = [f"```python\n{code}\n```", f"```python\nfinal_answer('''{code}''')\n```"]
        registry = ProviderRegistry([])
        registry.register("s", ScriptedProvider(script, model="s"))
        sandbox = Sandbox(config=SandboxConfig(allowed_directories=[workdir]),
                          tools=MBPPTools(test_list=task.test_list).registry())
        loop = AgentLoop(sandbox, registry, model="s", config=AgentConfig(max_iterations=3))
        return loop.run(task)

    report = evaluate(tasks, runner)
    assert report.total == 3
    assert report.passed == 3
    assert report.pass_at_1 == 1.0


def test_evaluation_detects_a_failure(workdir):
    task = Task(task_id="t1", benchmark="mbpp", prompt="p", test_list=["assert f() == 1"])

    def runner(task):
        # The model never submits: the run stops on the iteration limit.
        registry = ProviderRegistry([])
        registry.register("s", ScriptedProvider(["```python\nx = 1\n```"] * 5, model="s"))
        sandbox = Sandbox(config=SandboxConfig(allowed_directories=[workdir]))
        loop = AgentLoop(sandbox, registry, model="s", config=AgentConfig(max_iterations=2))
        return loop.run(task)

    report = evaluate([task], runner)
    assert report.passed == 0
    assert report.failed == 1
    assert "iteration limit" in report.failures()[0].error


# -- CLI -------------------------------------------------------------------
def _run_cli(args, cwd=ROOT):
    return subprocess.run([sys.executable, CLI] + args, capture_output=True, text=True,
                          cwd=cwd, timeout=120)


def test_cli_help():
    result = _run_cli(["--help"])
    assert result.returncode == 0
    assert "run" in result.stdout
    assert "sandbox" in result.stdout


def test_cli_lists_mbpp_tools():
    result = _run_cli(["tools", "--mcp", "mbpp"])
    assert result.returncode == 0
    assert "run_tests" in result.stdout
    assert "check_syntax" in result.stdout


def test_cli_tools_manual_flag():
    result = _run_cli(["tools", "--mcp", "mbpp", "--manual"])
    assert "Available tools" in result.stdout


def test_cli_run_with_a_scripted_model(tmp_path):
    task_file = tmp_path / "task.json"
    task_file.write_text(json.dumps({
        "task_id": "cli_1", "benchmark": "mbpp", "prompt": "write add",
        "test_list": ["assert add(1, 2) == 3"],
    }), encoding="utf-8")
    script_file = tmp_path / "script.json"
    script_file.write_text(json.dumps({"script": [
        f"```python\n{GOOD}\n```",
        f"```python\nfinal_answer('''{GOOD}''')\n```",
    ]}), encoding="utf-8")
    output = tmp_path / "solution.json"

    result = _run_cli([
        "run", "--benchmark", "mbpp", "--task-file", str(task_file),
        "--output", str(output), "--scripted", str(script_file),
    ])
    assert result.returncode == 0, result.stderr
    payload = json.loads(output.read_text())
    assert payload["success"] is True
    assert payload["task_id"] == "cli_1"


def test_cli_run_returns_a_non_zero_exit_on_failure(tmp_path):
    task_file = tmp_path / "task.json"
    task_file.write_text(json.dumps({
        "task_id": "cli_2", "benchmark": "mbpp", "prompt": "never submit",
    }), encoding="utf-8")
    script_file = tmp_path / "script.json"
    script_file.write_text(json.dumps({"script": ["```python\nx = 1\n```"] * 5}), encoding="utf-8")
    result = _run_cli([
        "run", "--benchmark", "mbpp", "--task-file", str(task_file),
        "--output", str(tmp_path / "s.json"), "--scripted", str(script_file),
        "--max-iterations", "2",
    ])
    assert result.returncode == 1


def test_cli_run_missing_task_file():
    result = _run_cli(["run", "--benchmark", "mbpp", "--task-file", "/nope/x.json"])
    assert result.returncode != 0
    assert "not found" in (result.stderr + result.stdout)


def test_cli_evaluate(tmp_path):
    tasks_file = tmp_path / "tasks.json"
    tasks_file.write_text(json.dumps({"tasks": [
        {"task_id": "e1", "benchmark": "mbpp", "prompt": "p",
         "test_list": ["assert add(1, 2) == 3"]},
    ]}), encoding="utf-8")
    script_file = tmp_path / "script.json"
    script_file.write_text(json.dumps({"script": [
        f"```python\n{GOOD}\n```",
        f"```python\nfinal_answer('''{GOOD}''')\n```",
    ]}), encoding="utf-8")
    report = tmp_path / "report.json"

    result = _run_cli([
        "evaluate", "--benchmark", "mbpp", "--tasks-file", str(tasks_file),
        "--scripted", str(script_file), "--output", str(report),
    ])
    assert result.returncode == 0, result.stderr
    assert "pass@1" in result.stdout
    assert json.loads(report.read_text())["passed"] == 1


def test_cli_sandbox_repl_executes_and_exits():
    result = subprocess.run(
        [sys.executable, CLI, "sandbox"],
        input="print('repl ok')\nexit\n", capture_output=True, text=True,
        cwd=ROOT, timeout=120,
    )
    assert "repl ok" in result.stdout


def test_cli_sandbox_repl_with_tools():
    result = subprocess.run(
        [sys.executable, CLI, "sandbox", "--mcp", "mbpp"],
        input="print(check_syntax('x = 1'))\nexit\n", capture_output=True, text=True,
        cwd=ROOT, timeout=120,
    )
    assert "parses" in result.stdout


def test_cli_rejects_unknown_benchmark():
    result = _run_cli(["run", "--benchmark", "humaneval", "--task-file", "x.json"])
    assert result.returncode != 0


def test_example_data_files_are_valid():
    data_dir = os.path.join(ROOT, "data")
    for name in ("sandbox.json", "mbpp_task.example.json", "swe_task.example.json",
                 "scripted_mbpp.json", "scripted_swebench.json",
                 "models.example.json", "mbpp_bench.example.json"):
        path = os.path.join(data_dir, name)
        with open(path, encoding="utf-8") as fh:
            json.load(fh)


def test_example_sandbox_config_loads():
    from config import load_config

    sandbox, agent = load_config(os.path.join(ROOT, "data", "sandbox.json"))
    assert sandbox.max_memory_mb == 512
    assert agent.max_iterations == 10


def test_example_models_file_builds_a_registry():
    from llm import ProviderRegistry

    path = os.path.join(ROOT, "data", "models.example.json")
    data = json.loads(open(path, encoding="utf-8").read())
    registry = ProviderRegistry.from_dict(data, env={})
    assert len(registry.names()) == 2
