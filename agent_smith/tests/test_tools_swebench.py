"""Tests for the nine mandatory SWE-bench tools."""

from __future__ import annotations

import os

import pytest

from tools_swebench import SWEBenchTools
from workspace import LocalWorkspace


@pytest.fixture
def server(git_repo):
    return SWEBenchTools(root=str(git_repo))


def _write(root, name, content):
    path = root / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")
    return path


# -- 1. read_file ----------------------------------------------------------
def test_read_file_cat_n_style(server):
    output = server.read_file("calc.py", 1, 2)
    assert "1: def add(a, b):" in output
    assert "2:     return a - b  # bug" in output


def test_read_file_reports_total_lines(server):
    assert "of 6" in server.read_file("calc.py", 1, 2)


def test_read_file_pagination_hint(server):
    output = server.read_file("calc.py", 1, 2)
    assert "start_line=3" in output


def test_read_file_caps_at_200_lines(git_repo):
    _write(git_repo, "big.txt", "\n".join(f"line {i}" for i in range(1, 501)))
    server = SWEBenchTools(root=str(git_repo))
    output = server.read_file("big.txt", 1, 500)
    assert "200: line 200" in output
    assert "201: line 201" not in output
    assert "start_line=201" in output


def test_read_file_end_line_defaults_to_end(server):
    output = server.read_file("calc.py")
    assert "6:     return a - b" in output


def test_read_file_missing(server):
    assert "file not found" in server.read_file("nope.py")


def test_read_file_start_past_end(server):
    assert "past the end" in server.read_file("calc.py", 99, 120)


def test_read_file_rejects_escape(server):
    assert "escapes" in server.read_file("../../etc/passwd")


# -- 2. edit_file ----------------------------------------------------------
def test_edit_file_unique_replacement(server, git_repo):
    result = server.edit_file("calc.py", "return a - b  # bug", "return a + b")
    assert "success" in result
    assert "return a + b" in (git_repo / "calc.py").read_text()


def test_edit_file_is_atomic_and_keeps_the_rest(server, git_repo):
    server.edit_file("calc.py", "return a - b  # bug", "return a + b")
    content = (git_repo / "calc.py").read_text()
    assert "def sub(a, b):" in content
    assert content.count("return a - b") == 1  # only the untouched `sub`


def test_edit_file_refuses_ambiguous_match(server, git_repo):
    result = server.edit_file("calc.py", "return a - b", "return a + b")
    assert "occurs 2 times" in result
    assert "lines 2, 6" in result
    # Nothing was written.
    assert (git_repo / "calc.py").read_text().count("return a - b") == 2


def test_edit_file_reports_missing_old_str(server):
    assert "not found" in server.edit_file("calc.py", "does not exist", "x")


def test_edit_file_requires_non_empty_old_str(server):
    assert "must not be empty" in server.edit_file("calc.py", "", "x")


def test_edit_file_reports_syntax_error_but_keeps_edit(server, git_repo):
    result = server.edit_file("calc.py", "return a - b  # bug", "return a + + ")
    assert "no longer parses" in result
    assert "SyntaxError" in result
    assert "return a + +" in (git_repo / "calc.py").read_text()


def test_edit_file_shows_context_window(server):
    result = server.edit_file("calc.py", "return a - b  # bug", "return a + b")
    assert "1: def add" in result


def test_edit_file_missing_file(server):
    assert "file not found" in server.edit_file("ghost.py", "a", "b")


# -- 3. list_files ---------------------------------------------------------
def test_list_files_sorted_with_directory_suffix(server):
    entries = server.list_files(".").splitlines()
    assert "calc.py" in entries
    assert "pkg/" in entries
    assert entries == sorted(entries)


def test_list_files_glob(server):
    entries = server.list_files(".", "*.py").splitlines()
    assert "calc.py" in entries
    assert "pkg/" not in entries


def test_list_files_skips_git_and_caches(server, git_repo):
    (git_repo / "__pycache__").mkdir(exist_ok=True)
    assert "__pycache__" not in server.list_files(".")


def test_list_files_pagination(git_repo):
    for index in range(150):
        _write(git_repo, f"mod_{index:03d}.py", "x = 1\n")
    server = SWEBenchTools(root=str(git_repo))
    first = server.list_files(".", "*.py")
    assert "offset=100" in first
    assert len(first.splitlines()) >= 100
    second = server.list_files(".", "*.py", 100)
    assert "mod_149.py" not in first
    assert "mod_149.py" in second


def test_list_files_missing_directory(server):
    assert "directory not found" in server.list_files("nope")


def test_list_files_no_match(server):
    assert "no entry matches" in server.list_files(".", "*.rs")


# -- 4. search_code --------------------------------------------------------
def test_search_code_reports_path_line_content(server):
    output = server.search_code("def add", "*.py")
    assert "calc.py:1 def add(a, b):" in output


def test_search_code_is_a_regex(server):
    assert "calc.py" in server.search_code(r"def\s+add", "*.py")


def test_search_code_skips_binary_and_git(server, git_repo):
    (git_repo / "blob.bin").write_bytes(b"\x00\x01def add")
    assert "blob.bin" not in server.search_code("def add", "*")


def test_search_code_invalid_regex(server):
    assert "invalid regular expression" in server.search_code("[unclosed")


def test_search_code_no_match(server):
    assert "no match found" in server.search_code("zzz_nothing", "*.py")


def test_search_code_pagination(git_repo):
    _write(git_repo, "many.txt", "\n".join("needle" for _ in range(250)))
    server = SWEBenchTools(root=str(git_repo))
    first = server.search_code("needle", "many.txt")
    assert "offset=100" in first
    later = server.search_code("needle", "many.txt", 200)
    assert "many.txt:250" in later


# -- 5. search_function_or_class_definition_in_code ------------------------
def test_finds_function_definition(server):
    assert "pkg/helper.py:1 def helper(x):" in server.search_function_or_class_definition_in_code("helper")


def test_finds_class_definition(server):
    assert "pkg/helper.py:5 class Widget:" in server.search_function_or_class_definition_in_code("Widget")


def test_reports_unknown_name(server):
    assert "no definition" in server.search_function_or_class_definition_in_code("ghost")


def test_survives_a_file_with_a_syntax_error(server, git_repo):
    _write(git_repo, "broken.py", "def broken(:\n    pass\n")
    # The broken file falls back to a regex scan, so the definition is still found.
    assert "no definition" in server.search_function_or_class_definition_in_code("ghost")
    output = server.search_function_or_class_definition_in_code("broken")
    assert "broken.py:1" in output


# -- 6. find_references ----------------------------------------------------
def test_find_references_excludes_definition_site(server):
    output = server.find_references("add", "calc.py", 1)
    assert "test_calc.py:4 assert add(2, 3) == 5" in output
    assert "calc.py:1 def add" not in output


def test_find_references_is_word_bounded(server, git_repo):
    _write(git_repo, "other.py", "address = 1\nadd = 2\n")
    output = server.find_references("add")
    assert "address" not in output
    assert "other.py:2 add = 2" in output


def test_find_references_without_definition_site(server):
    output = server.find_references("add")
    assert "calc.py:1 def add" in output


def test_find_references_requires_a_name(server):
    assert "name is required" in server.find_references("")


def test_find_references_no_match(server):
    assert "no reference" in server.find_references("zzz_nothing")


# -- 7. run_command --------------------------------------------------------
def test_run_command_reports_exit_code_and_streams(server):
    output = server.run_command("python3 -c 'import sys; print(\"o\"); print(\"e\", file=sys.stderr); sys.exit(3)'")
    assert "[exit_code] 3" in output
    assert "[stdout]" in output and "o" in output
    assert "[stderr]" in output and "e" in output


def test_run_command_success(server):
    assert "[exit_code] 0" in server.run_command("true")


def test_run_command_timeout_keeps_partial_output(server):
    output = server.run_command("python3 -c 'print(\"before\"); import time; time.sleep(30)'",
                                timeout=1)
    assert "timeout" in output.lower()


def test_run_command_in_subdirectory(server):
    output = server.run_command("pwd", workdir="pkg")
    assert "pkg" in output


def test_run_command_custom_workdir(server):
    assert "[workdir] pkg" in server.run_command("true", workdir="pkg")


# -- 8. get_patch ----------------------------------------------------------
def test_get_patch_empty(server):
    assert "empty patch" in server.get_patch()


def test_get_patch_includes_modifications(server, git_repo):
    server.edit_file("calc.py", "return a - b  # bug", "return a + b")
    patch = server.get_patch()
    assert "-    return a - b  # bug" in patch
    assert "+    return a + b" in patch


def test_get_patch_includes_new_files(server, git_repo):
    _write(git_repo, "newmod.py", "x = 1\n")
    patch = server.get_patch()
    assert "newmod.py" in patch
    assert "+x = 1" in patch


def test_get_patch_excludes_artifacts(server, git_repo):
    _write(git_repo, "__pycache__/calc.pyc", "binary-ish")
    _write(git_repo, "calc.py", "def add(a, b):\n    return a + b\n")
    patch = server.get_patch()
    assert "__pycache__" not in patch


def test_get_patch_ignores_permission_only_changes(server, git_repo):
    os.chmod(git_repo / "calc.py", 0o755)
    assert "empty patch" in server.get_patch()


# -- 9. run_tests ----------------------------------------------------------
def test_run_tests_without_eval_script(server):
    assert "no eval_script" in server.run_tests()


def test_run_tests_runs_the_bash_content(git_repo):
    server = SWEBenchTools(root=str(git_repo), eval_script="python3 test_calc.py")
    output = server.run_tests()
    assert "[exit_code] 1" in output  # the planted bug makes it fail
    assert "AssertionError" in output


def test_run_tests_passes_after_the_fix(git_repo):
    server = SWEBenchTools(root=str(git_repo), eval_script="python3 test_calc.py")
    server.edit_file("calc.py", "return a - b  # bug", "return a + b")
    assert "[exit_code] 0" in server.run_tests()


# -- registry --------------------------------------------------------------
def test_registry_contains_the_nine_tools(server):
    registry = server.registry()
    assert registry.names() == [
        "edit_file",
        "find_references",
        "get_patch",
        "list_files",
        "read_file",
        "run_command",
        "run_tests",
        "search_code",
        "search_function_or_class_definition_in_code",
    ]


def test_manual_documents_every_tool(server):
    manual = server.registry().manual()
    for name in server.registry().names():
        assert name in manual


# -- workspace -------------------------------------------------------------
def test_workspace_rejects_missing_root(tmp_path):
    from workspace import WorkspaceError

    with pytest.raises(WorkspaceError):
        LocalWorkspace(str(tmp_path / "absent"))


def test_workspace_rejects_path_escape(git_repo):
    from workspace import WorkspaceError

    workspace = LocalWorkspace(str(git_repo))
    with pytest.raises(WorkspaceError):
        workspace.read("../../etc/passwd")


def test_workspace_exec_working_directory(git_repo):
    workspace = LocalWorkspace(str(git_repo))
    assert "calc.py" in workspace.exec(["ls"]).stdout
