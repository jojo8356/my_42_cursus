"""SWE-bench tool server — the nine mandatory tools.

Every tool runs against a repository through a backend with the same interface
(``exec``, ``read``, ``replace_file``, ``workdir``, ``cleanup``), so the tools do
not know whether they sit on a container or on a host checkout.

Contract highlights (from the subject):

  read_file           1-based inclusive bounds, 200-line cap with a pagination
                      hint, ``cat -n`` style output.
  edit_file           exact replace; refuses AMBIGUOUS matches and lists every
                      candidate line; atomic write; reports a Python file that no
                      longer parses, with the error and its line.
  list_files          glob patterns, sorted, "/" suffix on directories, 100-entry
                      pagination.
  search_code         regex, skips .git and binary files, 100-match pagination,
                      "path:line content".
  search_function_or_class_definition_in_code
                      AST-based, survives syntax errors in the tree.
  find_references     word-boundary search, excluding the definition site.
  run_command         stdout, stderr AND exit code; timeout kill with partial
                      output; tail-kept cap.
  get_patch           ``git -c core.fileMode=false diff`` with intent-to-add so
                      new files appear; permission-only changes excluded.
  run_tests           runs the task's eval_script with a 600s timeout.
"""

from __future__ import annotations

import ast
import fnmatch
import json
import os
import posixpath
import re
import shlex
from typing import Any, Dict, Iterable, List, Optional

from mcp import ToolRegistry
from workspace import LocalWorkspace, WorkspaceError

READ_LINE_CAP = 200
LIST_PAGE = 100
SEARCH_PAGE = 100
MAX_STD = 6000
BINARY_HINTS = (".pyc", ".so", ".o", ".a", ".zip", ".gz", ".png", ".jpg", ".pdf", ".whl")
SKIP_DIRS = {".git", "__pycache__", ".mypy_cache", ".pytest_cache", ".tox", ".venv", "node_modules"}


def _truncate(text: str, limit: int, keep: str = "tail") -> str:
    if len(text) <= limit:
        return text
    if keep == "tail":
        return f"...[truncated: {len(text) - limit} chars omitted]...\n" + text[-(limit):]
    return text[:limit] + f"\n...[truncated: {len(text) - limit} chars omitted]"


class SWEBenchTools:
    """The SWE-bench tool server, backed by a workspace."""

    def __init__(self, workspace: Optional[LocalWorkspace] = None,
                 root: Optional[str] = None,
                 eval_script: Optional[str] = None,
                 max_std: int = MAX_STD) -> None:
        if workspace is None:
            if root is None:
                raise WorkspaceError("either a workspace or a repository root is required")
            workspace = LocalWorkspace(root)
        self.workspace = workspace
        self.eval_script = eval_script
        self.max_std = max_std

    # -- helpers ---------------------------------------------------------
    def _abs(self, path: str) -> str:
        if not path:
            raise WorkspaceError("an empty path is not valid")
        if os.path.isabs(path):
            target = path
        else:
            target = os.path.join(self.workspace.workdir(), path)
        real = os.path.realpath(os.path.abspath(target))
        root = self.workspace.workdir()
        if real != root and not real.startswith(root + os.sep):
            raise WorkspaceError(f"path escapes the repository: {path}")
        return real

    def _rel(self, path: str) -> str:
        root = self.workspace.workdir()
        real = os.path.realpath(os.path.abspath(path))
        if real == root:
            return "."
        if real.startswith(root + os.sep):
            return os.path.relpath(real, root)
        return real

    def _read_text(self, path: str) -> str:
        return self.workspace.read(path)

    def _iter_files(self, directory: Optional[str] = None,
                    file_pattern: Optional[str] = None) -> Iterable[str]:
        root = self.workspace.workdir()
        base = self._abs(directory) if directory else root
        if os.path.isfile(base):
            yield base
            return
        for dirpath, dirnames, filenames in os.walk(base):
            dirnames[:] = sorted(d for d in dirnames if d not in SKIP_DIRS)
            for filename in sorted(filenames):
                if filename.endswith(BINARY_HINTS):
                    continue
                full = os.path.join(dirpath, filename)
                if file_pattern and not _match_pattern(full, file_pattern, root):
                    continue
                yield full

    # -- 1. read_file ----------------------------------------------------
    def read_file(self, filepath: str, start_line: int = 1,
                  end_line: Optional[int] = None) -> str:
        """Read a file: 1-based inclusive bounds, cat -n style, 200-line cap."""
        try:
            real = self._abs(filepath)
        except WorkspaceError as exc:
            return f"error: {exc}"
        if not os.path.isfile(real):
            return f"error: file not found: {filepath}"
        content = self._read_text(real)
        lines = content.splitlines()
        total = len(lines)
        start = max(1, int(start_line or 1))
        if start > total:
            return (
                f"error: start_line {start} is past the end of {self._rel(real)} "
                f"({total} lines)"
            )
        last = int(end_line) if end_line is not None else total
        last = min(last, total)
        capped = last - start + 1 > READ_LINE_CAP
        if capped:
            last = start + READ_LINE_CAP - 1

        rendered = [f"{number}: {lines[number - 1]}" for number in range(start, last + 1)]
        header = f"[{self._rel(real)} lines {start}-{last} of {total}]"
        footer = ""
        if last < total:
            footer = (
                f"\n[truncated: showing {last - start + 1} lines of {total}. "
                f"Call read_file again with start_line={last + 1} to continue.]"
            )
        return header + "\n" + "\n".join(rendered) + footer

    # -- 2. edit_file ----------------------------------------------------
    def edit_file(self, filepath: str, old_str: str, new_str: str) -> str:
        """Exact string replacement; refuses ambiguous matches; atomic write."""
        try:
            real = self._abs(filepath)
        except WorkspaceError as exc:
            return f"error: {exc}"
        if not os.path.isfile(real):
            return f"error: file not found: {filepath}"
        if not old_str:
            return "error: old_str must not be empty"
        content = self._read_text(real)
        occurrences = content.count(old_str)
        if occurrences == 0:
            return f"error: old_str was not found in {self._rel(real)}"
        if occurrences > 1:
            lines = _line_numbers(content, old_str)
            return (
                f"error: old_str occurs {occurrences} times in {self._rel(real)} "
                f"(lines {', '.join(map(str, lines))}). Make it unique."
            )

        updated = content.replace(old_str, new_str, 1)
        self.workspace.replace_file(real, updated)

        message = f"success: edited {self._rel(real)}"
        if real.endswith(".py"):
            try:
                ast.parse(updated)
            except SyntaxError as exc:
                message += (
                    f"\nwarning: the file no longer parses: SyntaxError: {exc.msg} "
                    f"(line {exc.lineno})"
                )
        return message + "\n" + self._context_window(updated, new_str, real)

    def _context_window(self, content: str, needle: str, path: str, context: int = 3) -> str:
        lines = content.splitlines()
        index = content.find(needle)
        if index == -1 or not needle:
            start, end = 1, min(len(lines), 6)
        else:
            first_line = content[:index].count("\n") + 1
            start = max(1, first_line - context)
            end = min(len(lines), first_line + needle.count("\n") + context)
        return "\n".join(f"{n}: {lines[n - 1]}" for n in range(start, end + 1))

    # -- 3. list_files ---------------------------------------------------
    def list_files(self, directory: str = ".", pattern: str = "*", offset: int = 0) -> str:
        """List files matching a glob, sorted, with "/" suffixes, paginated."""
        root = self.workspace.workdir()
        base = self._abs(directory) if directory not in (".", "") else root
        entries: List[str] = []
        if os.path.isfile(base):
            return self._rel(base)
        if not os.path.isdir(base):
            return f"error: directory not found: {directory}"
        for name in sorted(os.listdir(base)):
            full = os.path.join(base, name)
            if name in SKIP_DIRS:
                continue
            if not _match_name(name, pattern):
                continue
            rel = self._rel(full)
            entries.append(rel + "/" if os.path.isdir(full) else rel)
        offset = max(0, int(offset or 0))
        page = entries[offset:offset + LIST_PAGE]
        footer = ""
        if offset + LIST_PAGE < len(entries):
            footer = (
                f"\n[truncated: showing {len(page)} of {len(entries)} entries. "
                f"Call list_files again with offset={offset + LIST_PAGE}.]"
            )
        if not page:
            return f"no entry matches {pattern!r} in {self._rel(base)}" + footer
        return "\n".join(page) + footer

    # -- 4. search_code --------------------------------------------------
    def search_code(self, pattern: str, file_pattern: str = "*", offset: int = 0) -> str:
        """Regex search, skipping .git and binary files, paginated."""
        try:
            compiled = re.compile(pattern)
        except re.error as exc:
            return f"error: invalid regular expression: {exc}"
        matches: List[str] = []
        for path in self._iter_files(file_pattern=file_pattern):
            rel = self._rel(path)
            if rel.startswith(".git" + os.sep) or rel.startswith(".git/"):
                continue
            if _is_binary(path):
                continue
            try:
                with open(path, encoding="utf-8", errors="ignore") as fh:
                    for number, line in enumerate(fh, start=1):
                        if compiled.search(line):
                            matches.append(f"{rel}:{number} {line.rstrip()}")
            except OSError:
                continue
        return self._paginate(matches, offset, "search_code")

    def _paginate(self, matches: List[str], offset: int, tool: str) -> str:
        offset = max(0, int(offset or 0))
        page = matches[offset:offset + SEARCH_PAGE]
        if not page:
            base = "no match found" if not matches else f"error: offset {offset} is past the last match"
            return base
        text = "\n".join(page)
        if offset + SEARCH_PAGE < len(matches):
            text += (
                f"\n[truncated: showing {len(page)} of {len(matches)} matches. "
                f"Call {tool} again with offset={offset + SEARCH_PAGE}.]"
            )
        return _truncate(text, self.max_std)

    # -- 5. search_function_or_class_definition_in_code ------------------
    def search_function_or_class_definition_in_code(self, name: str,
                                                    offset: int = 0) -> str:
        """Find a function/class definition with AST (survives syntax errors)."""
        matches: List[str] = []
        for path in self._iter_files(file_pattern="*.py"):
            try:
                content = self._read_text(path)
            except (OSError, UnicodeDecodeError):
                continue
            try:
                tree = ast.parse(content)
            except SyntaxError:
                for number, line in _regex_fallback_definitions(content, name):
                    matches.append(f"{self._rel(path)}:{number} {line}")
                continue
            for node in ast.walk(tree):
                if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
                    if node.name == name:
                        lines = content.splitlines()
                        line = lines[node.lineno - 1] if node.lineno <= len(lines) else ""
                        matches.append(f"{self._rel(path)}:{node.lineno} {line.strip()}")
        if not matches:
            return f"no definition of {name!r} found"
        return self._paginate(sorted(matches), offset, "search_function_or_class_definition_in_code")

    # -- 6. find_references ----------------------------------------------
    def find_references(self, name: str, filepath: Optional[str] = None,
                        line: Optional[int] = None, offset: int = 0) -> str:
        """Word-boundary references across the workspace, minus the definition site."""
        if not name or not name.strip():
            return "error: a name is required"
        definition_path = self._rel(self._abs(filepath)) if filepath else None
        definition_line = int(line) if line else None
        boundary = re.compile(rf"(?<![\w]){re.escape(name)}(?![\w])")
        matches: List[str] = []
        for path in self._iter_files():
            rel = self._rel(path)
            if _is_binary(path):
                continue
            try:
                with open(path, encoding="utf-8", errors="ignore") as fh:
                    for number, text in enumerate(fh, start=1):
                        if not boundary.search(text):
                            continue
                        if definition_path is not None and rel == definition_path \
                                and definition_line is not None and number == definition_line:
                            continue
                        matches.append(f"{rel}:{number} {text.rstrip()}")
            except OSError:
                continue
        if not matches:
            return f"no reference to {name!r} found"
        return self._paginate(matches, offset, "find_references")

    # -- 7. run_command --------------------------------------------------
    def run_command(self, command: str, workdir: Optional[str] = None,
                    timeout: Optional[float] = None) -> str:
        """Run a shell command; report stdout, stderr and the exit code."""
        cwd = self._abs(workdir) if workdir else self.workspace.workdir()
        result = self.workspace.exec(command, workdir=cwd, timeout=timeout)
        stdout = _truncate(result.stdout, self.max_std)
        stderr = _truncate(result.stderr, self.max_std)
        sections = [
            f"[exit_code] {result.exit_code}",
            f"[workdir] {self._rel(cwd)}",
            "[stdout]\n" + (stdout.rstrip("\n") if stdout.strip() else "(empty)"),
            "[stderr]\n" + (stderr.rstrip("\n") if stderr.strip() else "(empty)"),
        ]
        if result.timed_out:
            sections.insert(0, "[timeout] the command exceeded its time limit and was killed")
        return "\n".join(sections)

    # -- 8. get_patch ----------------------------------------------------
    def get_patch(self) -> str:
        """Unified diff, intent-to-add included, permission-only changes excluded.

        Cache/build artefacts (``__pycache__``, ``*.pyc``, ``.venv``, ...) are
        dropped from the diff: they are generated, not part of the fix.
        """
        self.workspace.exec(["git", "add", "-N", "."])
        result = self.workspace.exec(["git", "-c", "core.fileMode=false", "diff"])
        if result.exit_code != 0:
            return f"error: git diff failed (exit {result.exit_code})\n{result.stderr.strip()}"
        patch = _filter_patch(result.stdout)
        if not patch.strip():
            return "(empty patch: no change in the repository)"
        return _truncate(patch, self.max_std * 4)

    # -- 9. run_tests ----------------------------------------------------
    def run_tests(self, timeout: float = 600) -> str:
        """Run the task's eval_script (its literal bash content, not a path)."""
        if not self.eval_script:
            return "error: no eval_script is available for this task"
        result = self.workspace.exec(self.eval_script, timeout=timeout)
        stdout = _truncate(result.stdout, self.max_std)
        stderr = _truncate(result.stderr, self.max_std)
        sections = [
            f"[exit_code] {result.exit_code}",
            "[stdout]\n" + (stdout.rstrip("\n") if stdout.strip() else "(empty)"),
            "[stderr]\n" + (stderr.rstrip("\n") if stderr.strip() else "(empty)"),
        ]
        if result.timed_out:
            sections.insert(0, "[timeout] the test suite exceeded its time limit")
        return "\n".join(sections)

    # -- registry --------------------------------------------------------
    def registry(self, name: str = "swebench") -> ToolRegistry:
        registry = ToolRegistry(name)
        registry.register(self.read_file, name="read_file")
        registry.register(self.edit_file, name="edit_file")
        registry.register(self.list_files, name="list_files")
        registry.register(self.search_code, name="search_code")
        registry.register(
            self.search_function_or_class_definition_in_code,
            name="search_function_or_class_definition_in_code",
        )
        registry.register(self.find_references, name="find_references")
        registry.register(self.run_command, name="run_command")
        registry.register(self.get_patch, name="get_patch")
        registry.register(self.run_tests, name="run_tests")
        return registry


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------
def _match_name(name: str, pattern: str) -> bool:
    return fnmatch.fnmatch(name, pattern)


def _match_pattern(full_path: str, pattern: str, root: str) -> bool:
    if not pattern or pattern == "*":
        return True
    rel = os.path.relpath(full_path, root)
    return (
        fnmatch.fnmatch(full_path, pattern)
        or fnmatch.fnmatch(rel, pattern)
        or fnmatch.fnmatch(os.path.basename(full_path), pattern)
    )


def _is_binary(path: str) -> bool:
    """True when the file has no NUL-free UTF-8 prefix (binary content)."""
    try:
        with open(path, "rb") as fh:
            chunk = fh.read(2048)
    except OSError:
        return True
    if b"\x00" in chunk:
        return True
    try:
        chunk.decode("utf-8")
    except UnicodeDecodeError:
        return True
    return False


def _is_artifact(path: str) -> bool:
    """True for generated files that must never appear in a submitted patch."""
    parts = path.replace("\\", "/").split("/")
    if any(part in SKIP_DIRS for part in parts):
        return True
    return path.endswith(BINARY_HINTS + (".pyo", ".orig", ".rej"))


def _filter_patch(patch: str) -> str:
    """Drop the file sections of a unified diff that only touch artefacts."""
    if not patch:
        return ""
    sections: List[str] = []
    current: List[str] = []
    keep = True
    for line in patch.splitlines(keepends=True):
        if line.startswith("diff --git "):
            if current and keep:
                sections.append("".join(current))
            current = [line]
            target = line.split(" b/", 1)[-1].strip()
            keep = not _is_artifact(target)
        else:
            current.append(line)
    if current and keep:
        sections.append("".join(current))
    return "".join(sections)


def _line_numbers(content: str, needle: str) -> List[int]:
    numbers: List[int] = []
    start = content.find(needle)
    while start != -1:
        numbers.append(content[:start].count("\n") + 1)
        start = content.find(needle, start + 1)
    return numbers


def _regex_fallback_definitions(content: str, name: str) -> List[tuple]:
    pattern = re.compile(
        rf"^\s*(?:async\s+def|def|class)\s+{re.escape(name)}\b", re.MULTILINE
    )
    found: List[tuple] = []
    for number, line in enumerate(content.splitlines(), start=1):
        if pattern.match(line):
            found.append((number, line.strip()))
    return found


def build_server(registry: ToolRegistry) -> ToolRegistry:
    """Identity helper kept for symmetry with the other servers."""
    return registry
