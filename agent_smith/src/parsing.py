"""Code extraction from raw LLM output.

The agent accepts several response formats and reports every liberty it takes
back to the model through ``Extraction.note`` so nothing happens silently:

  * a Python fenced block            ```python ... ```   (primary format)
  * a fence with no language tag     ``` ... ```          (note)
  * an unclosed fence                 ```python ...       (note)
  * a fence ending inside a string    ```python "a ``` b" (note)
  * Anthropic XML                     <invoke>...</invoke>  (note)
  * Hermes tool-call JSON             <tool_call>{...}</tool_call>  (note)
  * ReAct                             Action: ... / Action Input: {...}
  * code after a bare ``Code:``
  * several blocks: only the last one runs                 (note)

Non-Python tool calls are converted into Python calls, and the conversion is
named back to the model.
"""

from __future__ import annotations

import ast
import json
import re
from dataclasses import dataclass, field
from typing import Any, List, Optional, Tuple

FENCE_RE = re.compile(r"```([A-Za-z0-9_+\-]*)[ \t]*\n?(.*?)(?:```|\Z)", re.DOTALL)
INVOKE_RE = re.compile(r"<invoke\b[^>]*>(.*?)</invoke>", re.DOTALL)
TOOL_CALL_RE = re.compile(r"<tool_call>\s*(.*?)\s*</tool_call>", re.DOTALL)
REACT_ACTION_RE = re.compile(
    r"^[ \t]*Action[ \t]*:[ \t]*(?P<name>[A-Za-z_][\w\.\-]*)[ \t]*$"
    r"(?:[ \t]*\n[ \t]*Action[ \t]*Input[ \t]*:[ \t]*(?P<input>.*?)"
    r"(?=^[ \t]*(?:Action|Observation|Thought)[ \t]*:|\Z))?",
    re.DOTALL | re.MULTILINE,
)
CODE_LABEL_RE = re.compile(r"^\s*Code\s*:\s*\n?", re.MULTILINE)
XML_PARAM_RE = re.compile(
    r'<parameter\s+name="(?P<name>[^"]+)"[^>]*>(?P<value>.*?)</parameter>',
    re.DOTALL,
)

PYTHON_LANGS = {"", "python", "py", "python3", "py3"}


@dataclass
class Extraction:
    """Result of extracting executable code from a model response."""

    code: str = ""
    notes: List[str] = field(default_factory=list)
    language: str = ""
    raw_blocks: int = 0

    @property
    def found(self) -> bool:
        return bool(self.code.strip())

    @property
    def note(self) -> str:
        """Single-line note prefix, or empty string when nothing was assumed."""
        if not self.notes:
            return ""
        return " ".join(f"[note] {n}" for n in self.notes)

    def observation(self, observation: str) -> str:
        """Prefix ``observation`` with the notes, so the model sees them."""
        prefix = self.note
        if not prefix:
            return observation
        return prefix + "\n" + observation


def _strip_noise(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")


def _fence_end_is_inside_string(body: str) -> bool:
    """True when the block ``body`` is cut open by a fence inside a string.

    A block that ends inside a string literal cannot be parsed: Python then
    reports an unterminated string, an unexpected EOF, etc. That parser verdict is
    the oracle — it handles every quoting style, including triple quotes.
    """
    if not body or not body.strip():
        return False
    try:
        ast.parse(body)
        return False
    except SyntaxError as exc:
        message = (exc.msg or "").lower()
        return any(
            hint in message
            for hint in (
                "unterminated",
                "unexpected eof",
                "unexpected end",
                "eof while scanning",
                "was never closed",
                "invalid syntax",
            )
        )
    except (ValueError, MemoryError, RecursionError):
        return False


@dataclass
class _Block:
    """One fenced block found by :func:`_scan_blocks`."""

    language: str
    body: str
    closed: bool
    false_fence: bool = False


def _scan_blocks(text: str) -> List[_Block]:
    """Find every fenced block, ignoring fences that sit inside string literals.

    A closing fence that would cut a Python string in half (``print("a ``` b")``)
    is not a real end: the scan continues to the next fence on its own line.
    """
    blocks: List[_Block] = []
    position = 0
    length = len(text)
    while position < length:
        start = text.find("```", position)
        if start == -1:
            break
        line_start = text.rfind("\n", 0, start) + 1
        if text[line_start:start].strip():
            # A fence in the middle of a line is prose, not a delimiter.
            position = start + 3
            continue
        tag_end = text.find("\n", start)
        if tag_end == -1:
            language = text[start + 3:].strip().lower()
            blocks.append(_Block(language=language, body="", closed=False))
            break
        language = text[start + 3:tag_end].strip().lower()
        cursor = tag_end + 1
        body_start = cursor
        false_fence = False
        while True:
            close = text.find("```", cursor)
            if close == -1:
                blocks.append(_Block(language, text[body_start:], closed=False,
                                     false_fence=false_fence))
                return blocks
            # A real closing fence ends its line. A fence followed by more
            # text on the same line sits inside prose or inside a string
            # literal (`print("a ``` b")`), so the scan runs past it.
            after = text[close + 3:]
            line_tail = after.split("\n", 1)[0]
            if line_tail.strip():
                false_fence = True
                cursor = close + 3
                continue
            # A trailing newline is not part of the code: drop exactly one.
            candidate = text[body_start:close]
            if candidate.endswith("\n"):
                candidate = candidate[:-1]
            blocks.append(_Block(language, candidate, closed=True,
                                 false_fence=false_fence))
            position = close + 3
            break
    return blocks


def _extract_fenced(text: str, notes: List[str]) -> Tuple[str, str, int]:
    """Extract the LAST fenced block; record notes about format liberties."""
    blocks = _scan_blocks(text)
    if not blocks:
        return "", "", 0

    if len(blocks) > 1:
        notes.append(
            f"your answer contained {len(blocks)} code blocks; only the last one was executed."
        )

    chosen = blocks[-1]
    lang = chosen.language
    body = chosen.body

    if not chosen.closed:
        notes.append(
            "your code block was not closed with ``` — the remainder of the message was executed."
        )

    if lang not in PYTHON_LANGS:
        notes.append(
            f"your block was tagged `{lang}` but only Python is executed; the code was run as Python."
        )

    if chosen.false_fence:
        notes.append(
            "a ``` inside a string literal looked like the end of your block; the block was "
            "extended past it."
        )

    return body, lang, len(blocks)


def _react_to_code(name: str, raw_input: str) -> str:
    """Convert a ReAct ``Action`` / ``Action Input`` pair into a Python call."""
    args: Any = {}
    stripped = raw_input.strip()
    if stripped:
        try:
            args = json.loads(stripped)
        except json.JSONDecodeError:
            # Fall back to a python literal, then to a single positional string.
            try:
                args = ast.literal_eval(stripped)
            except (ValueError, SyntaxError):
                args = stripped
    return _call_to_code(name, args)


def _call_to_code(name: str, args: Any) -> str:
    """Render a tool call as Python source calling that tool."""
    tool = name.strip().replace("-", "_")
    if "." in tool:
        tool = tool.split(".")[-1]
    if isinstance(args, dict):
        kwargs = ", ".join(f"{k}={v!r}" for k, v in args.items())
        return f"{tool}({kwargs})"
    if isinstance(args, (list, tuple)):
        return f"{tool}({', '.join(repr(a) for a in args)})"
    if isinstance(args, str):
        return f"{tool}({args!r})"
    return f"{tool}()"


def _parse_invoke(body: str) -> Optional[Tuple[str, Any]]:
    """Parse an Anthropic ``<invoke>`` body into ``(tool_name, args)``."""
    name_match = re.search(r'<tool_name>\s*(.*?)\s*</tool_name>', body, re.DOTALL)
    if not name_match:
        return None
    name = name_match.group(1).strip()
    params = {m.group("name"): m.group("value").strip() for m in XML_PARAM_RE.finditer(body)}
    if not params:
        attr = re.search(r'(?:name|tool_name)\s*=\s*"([^"]+)"', body)
        if attr:
            name = attr.group(1)
    # Convert numeric / JSON-looking parameter values to real Python values.
    converted: dict = {}
    for key, value in params.items():
        try:
            converted[key] = json.loads(value)
        except json.JSONDecodeError:
            converted[key] = value
    return name, converted


def extract_code(response: str) -> Extraction:
    """Extract executable Python from a raw model response."""
    if response is None:
        return Extraction()
    text = _strip_noise(response)
    notes: List[str] = []

    # 1. Fenced blocks (the primary format).
    body, lang, blocks = _extract_fenced(text, notes)
    if body.strip():
        return Extraction(code=body.strip("\n"), notes=notes, language=lang or "python",
                          raw_blocks=blocks)

    # 2. Anthropic XML <invoke>.
    invoke = INVOKE_RE.search(text)
    if invoke:
        parsed = _parse_invoke(invoke.group(1))
        if parsed:
            name, args = parsed
            notes.append(
                f"your `<invoke>` call to `{name}` was converted into a Python call to `{name}`."
            )
            return Extraction(code=_call_to_code(name, args), notes=notes, language="xml")

    # 3. Hermes <tool_call> JSON.
    tool_call = TOOL_CALL_RE.search(text)
    if tool_call:
        payload = tool_call.group(1)
        try:
            data = json.loads(payload)
        except json.JSONDecodeError:
            data = None
        if isinstance(data, dict):
            name = data.get("name") or data.get("tool") or ""
            args = data.get("arguments", data.get("parameters", {}))
            if isinstance(args, str):
                try:
                    args = json.loads(args)
                except json.JSONDecodeError:
                    pass
            if name:
                notes.append(
                    f"your `<tool_call>` for `{name}` was converted into a Python call to `{name}`."
                )
                return Extraction(code=_call_to_code(name, args), notes=notes, language="tool_call")

    # 4. ReAct Action / Action Input.
    action = REACT_ACTION_RE.search(text)
    if action:
        name = action.group("name")
        raw_input = action.group("input") or ""
        notes.append(
            f"your ReAct `Action: {name}` was converted into a Python call to `{name}`."
        )
        return Extraction(code=_react_to_code(name, raw_input), notes=notes, language="react")

    # 5. Code after a bare `Code:`.
    label = CODE_LABEL_RE.search(text)
    if label:
        notes.append("your reply used a bare `Code:` label instead of a fenced block.")
        return Extraction(code=text[label.end():].strip("\n"), notes=notes, language="bare")

    return Extraction(notes=notes)


def looks_like_python(code: str) -> bool:
    """Best-effort check that ``code`` is syntactically valid Python."""
    try:
        ast.parse(code)
        return True
    except SyntaxError:
        return False


def count_code_blocks(response: str) -> int:
    """Number of fenced blocks in a response (used by the loop's diagnostics)."""
    return len(FENCE_RE.findall(_strip_noise(response or "")))
