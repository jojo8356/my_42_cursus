#!/usr/bin/env python3
"""Agent Smith — command line entry point.

    python3 agent_smith.py run        --benchmark mbpp --task-file TASK --output OUT
    python3 agent_smith.py sandbox    [--mcp mbpp|swebench] [--repo PATH]
    python3 agent_smith.py evaluate   --tasks-file TASKS [--output REPORT]
    python3 agent_smith.py tools      --mcp mbpp|swebench
"""

from __future__ import annotations

import argparse
import json
import os
import signal
import sys
from typing import Any, Dict, List, Optional

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "src"))

from agent_loop import AgentLoop  # noqa: E402
from config import AgentConfig, SandboxConfig, load_config  # noqa: E402
from evaluator import evaluate, load_tasks  # noqa: E402
from llm import (  # noqa: E402
    EchoProvider,
    ProviderRegistry,
    ScriptedProvider,
    normalize_url,
)
from mcp import ToolRegistry  # noqa: E402
from models import Task  # noqa: E402
from reporting import write_report, write_solution  # noqa: E402
from sandbox import Sandbox  # noqa: E402
from tools_mbpp import MBPPTools  # noqa: E402
from tools_swebench import SWEBenchTools  # noqa: E402
from workspace import LocalWorkspace, WorkspaceError  # noqa: E402


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def _read_json(path: str) -> Any:
    try:
        with open(path, encoding="utf-8") as fh:
            return json.load(fh)
    except FileNotFoundError:
        raise SystemExit(f"error: file not found: {path}") from None
    except json.JSONDecodeError as exc:
        raise SystemExit(f"error: {path} is not valid JSON: {exc}") from None


def build_tools(benchmark: str, task: Optional[Task] = None,
                repo: Optional[str] = None) -> ToolRegistry:
    """Build the tool server for ``benchmark``."""
    if benchmark == "mbpp":
        server = MBPPTools(test_list=task.test_list if task else None)
        return server.registry("mbpp")
    if benchmark == "swebench":
        root = repo or (task.repo_path if task else None) or os.getcwd()
        workspace = LocalWorkspace(root)
        server = SWEBenchTools(
            workspace=workspace,
            eval_script=task.eval_script if task else None,
        )
        return server.registry("swebench")
    raise SystemExit(f"error: unknown benchmark: {benchmark}")


def build_registry(args) -> ProviderRegistry:
    """Build the provider registry from the CLI arguments."""
    if args.scripted:
        payload = _read_json(args.scripted)
        script = payload.get("script", payload) if isinstance(payload, dict) else payload
        if not isinstance(script, list):
            raise SystemExit("error: a scripted file must contain a list of responses")
        provider = ScriptedProvider(script, model=args.model_name or "scripted-model",
                                    repeat_last=bool(payload.get("repeat_last"))
                                    if isinstance(payload, dict) else False)
        registry = ProviderRegistry([])
        registry.register(provider.model, provider)
        return registry

    if args.echo or not (args.provider_url or args.model_name):
        provider = EchoProvider(args.model_name or "echo-model")
        registry = ProviderRegistry([])
        registry.register(provider.model, provider)
        return registry

    models_file = args.models_file
    if models_file:
        data = _read_json(models_file)
        return ProviderRegistry.from_dict(data)

    provider_name = "custom"
    return ProviderRegistry([
        # One ad-hoc model described entirely by the flags.
        __import__("llm").ModelEntry(
            model=args.model_name,
            provider=provider_name,
            url=args.provider_url or "",
            key_variable="LLM_API_KEY",
            strength=0,
        )
    ])


def make_sandbox(benchmark: str, tools: ToolRegistry, sandbox_config: SandboxConfig,
                 repo: Optional[str] = None) -> Sandbox:
    directories = list(sandbox_config.allowed_directories)
    if repo and os.path.realpath(repo) not in directories:
        directories.append(os.path.realpath(repo))
    config = SandboxConfig.from_dict({**sandbox_config.to_dict(), "allowed_directories": directories})
    return Sandbox(config=config, tools=tools)


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------
def cmd_run(args) -> int:
    sandbox_config, agent_config = load_config(args.config)
    if args.max_iterations:
        agent_config = AgentConfig.from_dict({
            **agent_config.to_dict(), "max_iterations": args.max_iterations
        })
    task = Task.from_dict(_read_json(args.task_file))
    if task.benchmark != args.benchmark:
        print(f"warning: the task declares {task.benchmark!r}, running as {args.benchmark!r}",
              file=sys.stderr)

    tools = build_tools(args.benchmark, task=task, repo=args.repo)
    sandbox = make_sandbox(args.benchmark, tools, sandbox_config, repo=args.repo)
    registry = build_registry(args)
    loop = AgentLoop(sandbox, registry, model=registry.default_model() if registry.names() else "",
                     config=agent_config)

    def handle_signal(signum, frame):  # noqa: ARG001
        raise KeyboardInterrupt

    try:
        signal.signal(signal.SIGTERM, handle_signal)
    except (ValueError, OSError):
        pass

    try:
        solution = loop.run(task)
    except KeyboardInterrupt:
        print("interrupted: writing the partial trace", file=sys.stderr)
        return 130

    path = write_solution(solution, args.output)
    print(json.dumps({
        "success": solution.success,
        "steps": len(solution.steps),
        "error": solution.error,
        "output": path,
    }, indent=2))
    return 0 if solution.success else 1


def cmd_evaluate(args) -> int:
    data = _read_json(args.tasks_file)
    tasks = load_tasks(data)
    sandbox_config, agent_config = load_config(args.config)

    def runner(task: Task):
        tools = build_tools(task.benchmark, task=task, repo=args.repo)
        sandbox = make_sandbox(task.benchmark, tools, sandbox_config, repo=args.repo)
        registry = build_registry(args)
        loop = AgentLoop(sandbox, registry,
                         model=registry.default_model() if registry.names() else "",
                         config=agent_config)
        return loop.run(task)

    report = evaluate(tasks, runner, benchmark=args.benchmark)
    print(report.summary())
    if args.output:
        print(f"report written to {write_report(report, args.output)}")
    return 0 if report.failed == 0 else 1


def cmd_tools(args) -> int:
    tools = build_tools(args.benchmark, repo=args.repo)
    for name in tools.names():
        tool = tools.get(name)
        print(f"{tool.signature()}")
        if tool.description:
            print(f"    {tool.description}")
    if args.manual:
        print()
        print(tools.manual())
    return 0


def cmd_sandbox(args) -> int:
    sandbox_config, _ = load_config(args.config)
    directories = list(sandbox_config.allowed_directories)
    if args.repo:
        directories.append(os.path.realpath(args.repo))
    sandbox_config = SandboxConfig.from_dict({
        **sandbox_config.to_dict(),
        "allowed_directories": directories or [os.getcwd()],
    })
    tools: Optional[ToolRegistry] = None
    if args.mcp:
        tools = build_tools(args.mcp, repo=args.repo)
    sandbox = Sandbox(config=sandbox_config, tools=tools)

    if tools is not None:
        print(tools.manual())
    print("Agent Smith sandbox — type Python, `exit` or Ctrl+D to leave.")
    buffer: List[str] = []
    try:
        while True:
            try:
                line = input("... " if buffer else ">>> ")
            except EOFError:
                print()
                break
            if not buffer and line.strip() in ("exit", "quit"):
                break
            buffer.append(line)
            source = "\n".join(buffer)
            try:
                compile(source, "<repl>", "exec")
            except SyntaxError as exc:
                if exc.msg in ("unexpected EOF while parsing", "expected an indented block",
                               "unexpected EOF while parsing"):
                    continue
                print(f"SyntaxError: {exc.msg}")
                buffer = []
                continue
            result = sandbox.execute(source)
            buffer = []
            if result.stdout:
                print(result.stdout, end="")
            if result.stderr:
                print(result.stderr, end="", file=sys.stderr)
            if result.error:
                print(result.error)
            if result.timed_out:
                print("[timeout] the block was killed")
            if result.final_answer is not None:
                print(f"[final_answer] {result.final_answer!r}")
    except KeyboardInterrupt:
        print()
    finally:
        sandbox.close()
    return 0


# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="agent_smith.py", description="Agent Smith — autonomous coding agent"
    )
    parser.add_argument("--config", help="JSON configuration file (sandbox + agent)")
    parser.add_argument("--repo", help="repository path for the SWE-bench tools")
    sub = parser.add_subparsers(dest="command", required=True)

    def add_provider_flags(target) -> None:
        target.add_argument("--model-name", default="", help="model to use")
        target.add_argument("--provider-url", default="", help="provider base URL")
        target.add_argument("--models-file", default="", help="models.json provider registry")
        target.add_argument("--scripted", default="", help="JSON file of scripted responses")
        target.add_argument("--echo", action="store_true", help="use the offline echo provider")

    run = sub.add_parser("run", help="run the agent on one task")
    run.add_argument("--benchmark", choices=["mbpp", "swebench"], required=True)
    run.add_argument("--task-file", required=True)
    run.add_argument("--output", default="solution.json")
    run.add_argument("--max-iterations", type=int, default=0)
    add_provider_flags(run)

    ev = sub.add_parser("evaluate", help="evaluate a benchmark")
    ev.add_argument("--benchmark", choices=["mbpp", "swebench"], default="mbpp")
    ev.add_argument("--tasks-file", required=True)
    ev.add_argument("--output", default="")
    add_provider_flags(ev)

    tools = sub.add_parser("tools", help="list the tools of a server")
    tools.add_argument("--mcp", choices=["mbpp", "swebench"], default="mbpp")
    tools.add_argument("--manual", action="store_true", help="print the full manual")

    box = sub.add_parser("sandbox", help="interactive sandbox REPL")
    box.add_argument("--mcp", choices=["mbpp", "swebench"], default=None)
    return parser


def main(argv: Optional[List[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    if not hasattr(args, "benchmark"):
        args.benchmark = "mbpp"
    handlers = {
        "run": cmd_run,
        "evaluate": cmd_evaluate,
        "tools": cmd_tools,
        "sandbox": cmd_sandbox,
    }
    try:
        return handlers[args.command](args)
    except (ValueError, WorkspaceError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
