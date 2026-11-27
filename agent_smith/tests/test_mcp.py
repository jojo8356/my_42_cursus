"""Tests for the MCP-shaped tool registry."""

from __future__ import annotations

import json

import pytest

from mcp import Tool, ToolRegistry, build_registry


def echo(value: str):
    """Echo a value back."""
    return value


def add(a: int, b: int = 1):
    """Add two numbers.

    Longer explanation that must not end up in the one-line description.
    """
    return a + b


# -- registration ----------------------------------------------------------
def test_register_from_callable():
    registry = ToolRegistry("test")
    tool = registry.register(echo)
    assert tool.name == "echo"
    assert tool.description == "Echo a value back."
    assert "value: str" in tool.parameters["value"]


def test_decorator_registration():
    registry = ToolRegistry("test")

    @registry.tool(name="ping", description="Ping the server")
    def _ping():
        return "pong"

    assert registry.names() == ["ping"]
    assert registry.get("ping").description == "Ping the server"


def test_names_are_sorted():
    registry = build_registry("t", {"b": echo, "a": add})
    assert registry.names() == ["a", "b"]


def test_length_and_contains():
    registry = build_registry("t", {"a": echo})
    assert len(registry) == 1
    assert "a" in registry
    assert "z" not in registry


def test_defaults_are_documented():
    registry = build_registry("t", {"add": add})
    tool = registry.get("add")
    assert tool.parameters["b"] == "b: int=1"
    assert tool.required == ["a"]


def test_signature_rendering():
    registry = build_registry("t", {"add": add})
    assert registry.get("add").signature() == "add(a: int, b: int=1)"


def test_unknown_tool_raises():
    registry = build_registry("t", {"a": echo})
    with pytest.raises(KeyError, match="unknown tool"):
        registry.get("missing")


# -- manual ----------------------------------------------------------------
def test_manual_lists_every_tool():
    registry = build_registry("t", {"a": echo, "b": add})
    manual = registry.manual()
    assert "`a(value: str)`" in manual
    assert "`b(a: int, b: int=1)`" in manual
    assert "Echo a value back." in manual
    assert "Add two numbers." in manual


def test_manual_names_the_server():
    assert "`custom` MCP server" in ToolRegistry("custom").manual()


def test_manual_is_empty_but_valid_without_tools():
    assert "Available tools" in ToolRegistry("empty").manual()


# -- invocation ------------------------------------------------------------
def test_call_with_positional_arguments():
    registry = build_registry("t", {"add": add})
    assert registry.call("add", 2, 3) == 5


def test_call_with_keyword_arguments():
    registry = build_registry("t", {"add": add})
    assert registry.call("add", a=2) == 3


def test_call_reports_a_bad_signature():
    registry = build_registry("t", {"add": add})
    with pytest.raises(TypeError, match="add"):
        registry.call("add")


def test_call_json_wraps_a_failure():
    registry = build_registry("t", {"add": add})
    payload = json.loads(registry.call_json("add", {}))
    assert payload["success"] is False
    assert "error" in payload


def test_call_json_serialises_a_dict_result():
    registry = ToolRegistry("t")

    def info():
        """Return a dict."""
        return {"ok": True, "n": 1}

    registry.register(info)
    assert json.loads(registry.call_json("info", {})) == {"ok": True, "n": 1}


def test_call_json_passes_a_string_through():
    registry = build_registry("t", {"echo": echo})
    assert registry.call_json("echo", {"value": "raw"}) == "raw"


# -- serialisation ---------------------------------------------------------
def test_list_tools_shape():
    registry = build_registry("t", {"add": add})
    listed = registry.list_tools()
    assert listed[0]["name"] == "add"
    assert listed[0]["required"] == ["a"]
    assert "parameters" in listed[0]


def test_tool_to_dict():
    tool = Tool("x", "desc", echo, {"v": "v: str"}, ["v"])
    assert tool.to_dict()["description"] == "desc"


def test_add_existing_tool_object():
    registry = ToolRegistry("t")
    registry.add(Tool("x", "desc", echo))
    assert registry.names() == ["x"]


def test_build_registry_helper():
    registry = build_registry("srv", {"a": echo, "b": add})
    assert registry.name == "srv"
    assert len(registry) == 2
