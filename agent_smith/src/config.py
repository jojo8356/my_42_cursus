"""Validated configuration objects (SandboxConfig, AgentConfig).

The real project validates these with Pydantic; here the same constraints are
expressed in plain Python so the core stays dependency-free and the failure
messages stay equally explicit.
"""

from __future__ import annotations

import json
import os
from dataclasses import dataclass, field, replace
from typing import Any, Dict, Iterable, List, Optional

# Builtins removed from every sandbox namespace.
FORBIDDEN_BUILTINS: tuple = (
    "eval",
    "exec",
    "compile",
    "input",
    "breakpoint",
    "globals",
    "locals",
    "vars",
    "help",
    "exit",
    "quit",
    "memoryview",
    "setattr",
    "delattr",
    "super",
)

# Attributes that open the classic sandbox escape routes.
FORBIDDEN_ATTRIBUTES: tuple = (
    "gi_frame",
    "gi_code",
    "cr_frame",
    "cr_code",
    "ag_frame",
    "ag_code",
    "f_back",
    "f_globals",
    "f_locals",
    "f_builtins",
    "f_code",
    "tb_frame",
    "tb_next",
    "mro",
)

DEFAULT_AUTHORIZED_IMPORTS: List[str] = [
    "math",
    "cmath",
    "re",
    "json",
    "collections",
    "itertools",
    "functools",
    "string",
    "typing",
    "dataclasses",
    "enum",
    "fractions",
    "decimal",
    "statistics",
    "random",
    "heapq",
    "bisect",
    "copy",
    "unicodedata",
    "textwrap",
    "difflib",
    "operator",
    "numbers",
    "datetime",
    "time",
    "pathlib",
    "os.path",
    "sys",
]


def _as_list(value: Any, name: str) -> List[str]:
    if value is None:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, (list, tuple)):
        out = []
        for item in value:
            if not isinstance(item, str):
                raise ValueError(f"{name} entries must be strings, got {type(item).__name__}")
            out.append(item)
        return out
    raise ValueError(f"{name} must be a list of strings")


@dataclass
class SandboxConfig:
    """Limits and allowlists applied to generated code."""

    authorized_imports: List[str] = field(default_factory=lambda: list(DEFAULT_AUTHORIZED_IMPORTS))
    allowed_directories: List[str] = field(default_factory=list)
    forbidden_builtins: List[str] = field(default_factory=lambda: list(FORBIDDEN_BUILTINS))
    forbidden_attributes: List[str] = field(default_factory=lambda: list(FORBIDDEN_ATTRIBUTES))
    max_memory_mb: int = 512
    max_cpu_seconds: int = 30
    max_output_chars: int = 8000
    allow_network: bool = False
    allow_subprocess: bool = False
    persistence: bool = True

    def __post_init__(self) -> None:
        self.authorized_imports = _as_list(self.authorized_imports, "authorized_imports")
        self.allowed_directories = _as_list(self.allowed_directories, "allowed_directories")
        self.forbidden_builtins = _as_list(self.forbidden_builtins, "forbidden_builtins")
        self.forbidden_attributes = _as_list(self.forbidden_attributes, "forbidden_attributes")
        if self.max_memory_mb <= 0:
            raise ValueError("max_memory_mb must be positive")
        if self.max_cpu_seconds <= 0:
            raise ValueError("max_cpu_seconds must be positive")
        if self.max_output_chars <= 0:
            raise ValueError("max_output_chars must be positive")
        # Normalise allowed directories so later comparisons are real paths.
        self.allowed_directories = [
            os.path.realpath(os.path.abspath(d)) for d in self.allowed_directories if d
        ]

    # -- helpers ---------------------------------------------------------
    def import_allowed(self, module: str) -> bool:
        """True when ``module`` matches an allowlist entry (``pkg.*`` wildcards)."""
        if not module:
            return False
        for entry in self.authorized_imports:
            if entry.endswith(".*"):
                if module == entry[:-2] or module.startswith(entry[:-2] + "."):
                    return True
            elif module == entry:
                return True
            elif module.startswith(entry + "."):
                # `collections` also authorises `collections.abc`.
                return True
        return False

    def directory_allowed(self, path: str) -> bool:
        """True when the resolved ``path`` lives inside an allowed directory."""
        if not self.allowed_directories:
            return False
        try:
            real = os.path.realpath(os.path.abspath(path))
        except (OSError, ValueError):
            return False
        for allowed in self.allowed_directories:
            if real == allowed or real.startswith(allowed.rstrip(os.sep) + os.sep):
                return True
        return False

    def with_directories(self, *directories: str) -> "SandboxConfig":
        merged = list(self.allowed_directories) + [d for d in directories if d]
        return replace(self, allowed_directories=merged)

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "SandboxConfig":
        known = {f for f in cls.__dataclass_fields__}
        unknown = set(data) - known
        if unknown:
            raise ValueError(f"unknown SandboxConfig fields: {sorted(unknown)}")
        return cls(**data)

    @classmethod
    def load(cls, path: str) -> "SandboxConfig":
        try:
            with open(path, encoding="utf-8") as fh:
                data = json.load(fh)
        except FileNotFoundError:
            raise ValueError(f"config file not found: {path}") from None
        except json.JSONDecodeError as exc:
            raise ValueError(f"config file is not valid JSON ({path}): {exc}") from None
        if not isinstance(data, dict):
            raise ValueError(f"config file must contain a JSON object: {path}")
        # Allow either a bare SandboxConfig or {"sandbox": {...}}.
        if "sandbox" in data and isinstance(data["sandbox"], dict):
            data = data["sandbox"]
        return cls.from_dict(data)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "authorized_imports": list(self.authorized_imports),
            "allowed_directories": list(self.allowed_directories),
            "forbidden_builtins": list(self.forbidden_builtins),
            "forbidden_attributes": list(self.forbidden_attributes),
            "max_memory_mb": self.max_memory_mb,
            "max_cpu_seconds": self.max_cpu_seconds,
            "max_output_chars": self.max_output_chars,
            "allow_network": self.allow_network,
            "allow_subprocess": self.allow_subprocess,
            "persistence": self.persistence,
        }


@dataclass
class AgentConfig:
    """Budget and retry policy of one agent run."""

    max_iterations: int = 10
    max_input_tokens: int = 120_000
    max_output_tokens: int = 16_000
    max_wall_time_s: int = 600
    history_token_margin: float = 0.10
    retry_wait_s: float = 0.0
    submit_warning_iterations: int = 2
    submit_warning_ratio: float = 0.20

    def __post_init__(self) -> None:
        if self.max_iterations <= 0:
            raise ValueError("max_iterations must be positive")
        if self.max_input_tokens <= 0:
            raise ValueError("max_input_tokens must be positive")
        if self.max_output_tokens <= 0:
            raise ValueError("max_output_tokens must be positive")
        if self.max_wall_time_s <= 0:
            raise ValueError("max_wall_time_s must be positive")
        if not 0.0 <= self.history_token_margin < 1.0:
            raise ValueError("history_token_margin must be in [0, 1)")
        if not 0.0 <= self.submit_warning_ratio < 1.0:
            raise ValueError("submit_warning_ratio must be in [0, 1)")
        if self.retry_wait_s < 0:
            raise ValueError("retry_wait_s must not be negative")

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "AgentConfig":
        known = {f for f in cls.__dataclass_fields__}
        unknown = set(data) - known
        if unknown:
            raise ValueError(f"unknown AgentConfig fields: {sorted(unknown)}")
        return cls(**data)

    def to_dict(self) -> Dict[str, Any]:
        return {f: getattr(self, f) for f in self.__dataclass_fields__}


def load_config(path: Optional[str] = None) -> tuple:
    """Load a ``(SandboxConfig, AgentConfig)`` pair, with defaults when no path."""
    sandbox = SandboxConfig()
    agent = AgentConfig()
    if path is None:
        return sandbox, agent
    try:
        with open(path, encoding="utf-8") as fh:
            data = json.load(fh)
    except FileNotFoundError:
        raise ValueError(f"config file not found: {path}") from None
    except json.JSONDecodeError as exc:
        raise ValueError(f"config file is not valid JSON ({path}): {exc}") from None
    if not isinstance(data, dict):
        raise ValueError(f"config file must contain a JSON object: {path}")
    if "sandbox" in data:
        sandbox = SandboxConfig.from_dict(data["sandbox"])
    if "agent" in data:
        agent = AgentConfig.from_dict(data["agent"])
    return sandbox, agent
