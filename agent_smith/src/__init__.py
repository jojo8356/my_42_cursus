"""Agent Smith — an autonomous coding agent (Thought -> Code -> Observation).

The modules are also usable as plain top-level modules (``import sandbox``) when
``src/`` itself is on ``sys.path``, which is how ``agent_smith.py`` loads them.
"""

from __future__ import annotations

__version__ = "1.0.0"

try:  # package mode: ``from agent_smith.src import ...``
    from .config import AgentConfig, SandboxConfig, load_config
    from .models import ExecutionResult, SolutionOutput, StepMetrics, Task, ToolCall
except ImportError:  # flat mode: ``src/`` on sys.path
    from config import AgentConfig, SandboxConfig, load_config
    from models import ExecutionResult, SolutionOutput, StepMetrics, Task, ToolCall

__all__ = [
    "AgentConfig",
    "SandboxConfig",
    "load_config",
    "ExecutionResult",
    "SolutionOutput",
    "StepMetrics",
    "Task",
    "ToolCall",
    "__version__",
]
