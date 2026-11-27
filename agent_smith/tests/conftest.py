"""Shared fixtures for the Agent Smith test suite."""

from __future__ import annotations

import os
import subprocess
import sys

import pytest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")
for path in (SRC, ROOT):
    if path not in sys.path:
        sys.path.insert(0, path)

from config import AgentConfig, SandboxConfig  # noqa: E402
from models import Task  # noqa: E402


@pytest.fixture
def repo_root():
    """Absolute path of the project (an allowed sandbox directory)."""
    return os.path.realpath(ROOT)


@pytest.fixture
def workdir(tmp_path):
    """An empty allowed directory for filesystem tests."""
    directory = tmp_path / "allowed"
    directory.mkdir()
    return str(directory)


@pytest.fixture
def sandbox_config(workdir):
    return SandboxConfig(
        allowed_directories=[workdir],
        max_cpu_seconds=3,
        max_memory_mb=512,
        max_output_chars=2000,
    )


@pytest.fixture
def sandbox(sandbox_config):
    from sandbox import Sandbox

    box = Sandbox(config=sandbox_config)
    yield box
    box.close()


@pytest.fixture
def agent_config():
    return AgentConfig(max_iterations=5, max_input_tokens=50_000,
                       max_output_tokens=5_000, max_wall_time_s=60)


@pytest.fixture
def mbpp_task():
    return Task(
        task_id="mbpp_test_1",
        benchmark="mbpp",
        prompt="Write `add(a, b)` returning a + b.",
        entry_point="add",
        test_list=["assert add(1, 2) == 3", "assert add(-1, 1) == 0"],
    )


@pytest.fixture
def git_repo(tmp_path):
    """A small git repository with a planted bug."""
    root = tmp_path / "repo"
    root.mkdir()
    (root / "calc.py").write_text(
        "def add(a, b):\n"
        "    return a - b  # bug\n"
        "\n"
        "\n"
        "def sub(a, b):\n"
        "    return a - b\n",
        encoding="utf-8",
    )
    (root / "test_calc.py").write_text(
        "import sys\nsys.path.insert(0, '.')\nfrom calc import add\nassert add(2, 3) == 5\n",
        encoding="utf-8",
    )
    (root / "pkg").mkdir()
    (root / "pkg" / "__init__.py").write_text("", encoding="utf-8")
    (root / "pkg" / "helper.py").write_text(
        "def helper(x):\n    return x * 2\n\n\nclass Widget:\n    def render(self):\n        return 'w'\n",
        encoding="utf-8",
    )
    (root / ".gitignore").write_text("__pycache__/\n", encoding="utf-8")
    subprocess.run(["git", "init", "-q", str(root)], check=True)
    subprocess.run(["git", "-C", str(root), "config", "user.email", "t@example.com"], check=True)
    subprocess.run(["git", "-C", str(root), "config", "user.name", "tester"], check=True)
    subprocess.run(["git", "-C", str(root), "add", "-A"], check=True)
    subprocess.run(["git", "-C", str(root), "commit", "-qm", "init"], check=True)
    return root
