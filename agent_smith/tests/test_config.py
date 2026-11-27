"""Tests for SandboxConfig / AgentConfig and their loaders."""

from __future__ import annotations

import json
import os

import pytest

from config import (
    AgentConfig,
    SandboxConfig,
    load_config,
)


# -- SandboxConfig ---------------------------------------------------------
def test_default_sandbox_is_locked_down():
    config = SandboxConfig()
    assert config.allow_network is False
    assert config.allow_subprocess is False
    assert config.max_memory_mb > 0
    assert "math" in config.authorized_imports
    assert "eval" in config.forbidden_builtins


def test_import_allowlist_exact_and_wildcard():
    config = SandboxConfig(authorized_imports=["math", "pkg.*"])
    assert config.import_allowed("math")
    assert config.import_allowed("pkg")
    assert config.import_allowed("pkg.sub")
    assert config.import_allowed("pkg.sub.deep")
    assert not config.import_allowed("os")
    assert not config.import_allowed("pkgx")
    assert not config.import_allowed("")


def test_import_allowlist_submodule_of_allowed_root():
    config = SandboxConfig(authorized_imports=["collections"])
    assert config.import_allowed("collections.abc")


def test_directory_allowed_inside_and_outside(workdir, tmp_path):
    config = SandboxConfig(allowed_directories=[workdir])
    assert config.directory_allowed(os.path.join(workdir, "a.txt"))
    assert config.directory_allowed(workdir)
    assert not config.directory_allowed(str(tmp_path / "elsewhere.txt"))
    assert not config.directory_allowed("/etc/passwd")


def test_directory_allowed_rejects_traversal(workdir):
    config = SandboxConfig(allowed_directories=[workdir])
    traversal = os.path.join(workdir, "..", "..", "etc", "passwd")
    assert not config.directory_allowed(traversal)


def test_no_allowed_directory_denies_everything(workdir):
    config = SandboxConfig()
    assert not config.directory_allowed(workdir)


def test_sandbox_rejects_non_positive_limits():
    with pytest.raises(ValueError, match="max_memory_mb"):
        SandboxConfig(max_memory_mb=0)
    with pytest.raises(ValueError, match="max_cpu_seconds"):
        SandboxConfig(max_cpu_seconds=-1)
    with pytest.raises(ValueError, match="max_output_chars"):
        SandboxConfig(max_output_chars=0)


def test_sandbox_rejects_non_string_lists():
    with pytest.raises(ValueError, match="must be strings"):
        SandboxConfig(authorized_imports=[1, 2])


def test_sandbox_accepts_single_string():
    config = SandboxConfig(authorized_imports="math")
    assert config.authorized_imports == ["math"]


def test_with_directories_merges():
    config = SandboxConfig(allowed_directories=["/tmp/a"])
    merged = config.with_directories("/tmp/b")
    assert "/tmp/b" in merged.allowed_directories
    assert len(merged.allowed_directories) == 2
    assert len(config.allowed_directories) == 1  # original untouched


def test_sandbox_from_dict_rejects_unknown_field():
    with pytest.raises(ValueError, match="unknown SandboxConfig"):
        SandboxConfig.from_dict({"max_memory_mb": 64, "teleport": True})


def test_sandbox_from_dict_round_trip(workdir):
    config = SandboxConfig(allowed_directories=[workdir], max_cpu_seconds=7)
    clone = SandboxConfig.from_dict(config.to_dict())
    assert clone.max_cpu_seconds == 7
    assert clone.allowed_directories == config.allowed_directories


def test_sandbox_load_nested_and_flat(tmp_path):
    nested = tmp_path / "nested.json"
    nested.write_text(json.dumps({"sandbox": {"max_cpu_seconds": 11}}), encoding="utf-8")
    assert SandboxConfig.load(str(nested)).max_cpu_seconds == 11

    flat = tmp_path / "flat.json"
    flat.write_text(json.dumps({"max_cpu_seconds": 12}), encoding="utf-8")
    assert SandboxConfig.load(str(flat)).max_cpu_seconds == 12


def test_sandbox_load_errors(tmp_path):
    with pytest.raises(ValueError, match="not found"):
        SandboxConfig.load(str(tmp_path / "missing.json"))
    broken = tmp_path / "broken.json"
    broken.write_text("{nope", encoding="utf-8")
    with pytest.raises(ValueError, match="not valid JSON"):
        SandboxConfig.load(str(broken))


# -- AgentConfig -----------------------------------------------------------
def test_agent_config_defaults():
    config = AgentConfig()
    assert config.max_iterations > 0
    assert 0 <= config.history_token_margin < 1


@pytest.mark.parametrize("field,value", [
    ("max_iterations", 0),
    ("max_input_tokens", 0),
    ("max_output_tokens", -5),
    ("max_wall_time_s", 0),
])
def test_agent_config_rejects_bad_budgets(field, value):
    with pytest.raises(ValueError):
        AgentConfig(**{field: value})


def test_agent_config_rejects_bad_margin():
    with pytest.raises(ValueError, match="history_token_margin"):
        AgentConfig(history_token_margin=1.0)
    with pytest.raises(ValueError, match="submit_warning_ratio"):
        AgentConfig(submit_warning_ratio=-0.1)


def test_agent_config_round_trip():
    config = AgentConfig(max_iterations=3)
    assert AgentConfig.from_dict(config.to_dict()).max_iterations == 3


def test_agent_config_rejects_unknown_field():
    with pytest.raises(ValueError, match="unknown AgentConfig"):
        AgentConfig.from_dict({"max_iterations": 2, "magic": 1})


# -- load_config -----------------------------------------------------------
def test_load_config_without_path_returns_defaults():
    sandbox, agent = load_config(None)
    assert isinstance(sandbox, SandboxConfig)
    assert isinstance(agent, AgentConfig)
    assert sandbox.max_memory_mb == 512


def test_load_config_from_file(workdir, tmp_path):
    path = tmp_path / "config.json"
    path.write_text(json.dumps({
        "sandbox": {"allowed_directories": [workdir], "max_cpu_seconds": 4},
        "agent": {"max_iterations": 7},
    }), encoding="utf-8")
    sandbox, agent = load_config(str(path))
    assert sandbox.max_cpu_seconds == 4
    assert sandbox.allowed_directories == [os.path.realpath(workdir)]
    assert agent.max_iterations == 7


def test_load_config_missing_file(tmp_path):
    with pytest.raises(ValueError, match="not found"):
        load_config(str(tmp_path / "absent.json"))


def test_load_config_invalid_json(tmp_path):
    path = tmp_path / "bad.json"
    path.write_text("[]", encoding="utf-8")
    with pytest.raises(ValueError, match="JSON object"):
        load_config(str(path))
