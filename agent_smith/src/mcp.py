"""MCP-shaped tool registry.

The agent loop never hardcodes a tool name: it receives a server, lists its tools
and generates the manual from their signatures. Connecting a different server
changes both the available functions and the generated manual automatically.
"""

from __future__ import annotations

import inspect
import json
from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Optional


@dataclass
class Tool:
    """One tool exposed to the model."""

    name: str
    description: str
    func: Callable[..., Any]
    parameters: Dict[str, str] = field(default_factory=dict)
    required: List[str] = field(default_factory=list)

    @classmethod
    def from_callable(cls, func: Callable[..., Any], name: Optional[str] = None,
                      description: Optional[str] = None) -> "Tool":
        signature = inspect.signature(func)
        parameters: Dict[str, str] = {}
        required: List[str] = []
        for param_name, param in signature.parameters.items():
            annotation = ""
            if param.annotation is not inspect.Parameter.empty:
                # With `from __future__ import annotations` the annotation is the
                # source string ("str", "int | None"): keep it as written.
                raw = param.annotation
                annotation = raw if isinstance(raw, str) else getattr(
                    raw, "__name__", str(raw)
                )
            default = ""
            if param.default is not inspect.Parameter.empty:
                default = f"={param.default!r}"
            rendered = f"{param_name}: {annotation}" if annotation else param_name
            parameters[param_name] = f"{rendered}{default}"
            if param.default is inspect.Parameter.empty:
                required.append(param_name)
        doc = description or inspect.getdoc(func) or ""
        return cls(
            name=name or func.__name__,
            description=doc.strip().split("\n\n")[0].strip(),
            func=func,
            parameters=parameters,
            required=required,
        )

    def signature(self) -> str:
        return f"{self.name}({', '.join(self.parameters.values())})"

    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "description": self.description,
            "parameters": dict(self.parameters),
            "required": list(self.required),
        }

    def call(self, *args: Any, **kwargs: Any) -> Any:
        return self.func(*args, **kwargs)


class ToolRegistry:
    """An MCP server: a named collection of tools with a generated manual."""

    def __init__(self, name: str = "tools") -> None:
        self.name = name
        self._tools: Dict[str, Tool] = {}

    # -- registration ----------------------------------------------------
    def register(self, func: Callable[..., Any], name: Optional[str] = None,
                 description: Optional[str] = None) -> Tool:
        tool = Tool.from_callable(func, name=name, description=description)
        self._tools[tool.name] = tool
        return tool

    def add(self, tool: Tool) -> Tool:
        self._tools[tool.name] = tool
        return tool

    def tool(self, name: Optional[str] = None, description: Optional[str] = None):
        """Decorator form of :meth:`register`."""

        def decorator(func: Callable[..., Any]) -> Callable[..., Any]:
            self.register(func, name=name, description=description)
            return func

        return decorator

    # -- introspection ---------------------------------------------------
    def names(self) -> List[str]:
        return sorted(self._tools)

    def get(self, name: str) -> Tool:
        if name not in self._tools:
            raise KeyError(f"unknown tool: {name!r} (available: {', '.join(self.names())})")
        return self._tools[name]

    def __contains__(self, name: object) -> bool:
        return name in self._tools

    def __len__(self) -> int:
        return len(self._tools)

    def list_tools(self) -> List[Dict[str, Any]]:
        return [self._tools[name].to_dict() for name in self.names()]

    def manual(self) -> str:
        """Human/model-readable description of every tool."""
        lines = [f"## Available tools (`{self.name}` MCP server)\n"]
        for name in self.names():
            tool = self._tools[name]
            lines.append(f"### `{tool.signature()}`")
            if tool.description:
                lines.append(tool.description)
            lines.append("")
        return "\n".join(lines).rstrip() + "\n"

    # -- invocation ------------------------------------------------------
    def call(self, name: str, *args: Any, **kwargs: Any) -> Any:
        tool = self.get(name)
        try:
            return tool.call(*args, **kwargs)
        except TypeError as exc:
            raise TypeError(f"{name}: {exc}") from exc

    def call_json(self, name: str, arguments: Dict[str, Any]) -> str:
        """Call a tool from a JSON payload and return a JSON string response."""
        try:
            result = self.call(name, **arguments)
        except Exception as exc:  # noqa: BLE001 - MCP servers report, never raise
            return json.dumps({"success": False, "error": f"{type(exc).__name__}: {exc}"})
        if isinstance(result, str):
            return result
        return json.dumps(result, default=str)


def build_registry(name: str, funcs: Dict[str, Callable[..., Any]]) -> ToolRegistry:
    """Build a registry from a mapping name -> callable."""
    registry = ToolRegistry(name)
    for tool_name, func in funcs.items():
        registry.register(func, name=tool_name)
    return registry
