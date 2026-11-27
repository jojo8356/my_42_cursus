"""Tests for code extraction: every accepted format and every note."""

from __future__ import annotations

import pytest

from parsing import (
    Extraction,
    count_code_blocks,
    extract_code,
    looks_like_python,
)


# -- primary format --------------------------------------------------------
def test_extracts_python_fence():
    response = "Thought: fine.\n```python\nx = 1\nprint(x)\n```\nDone."
    result = extract_code(response)
    assert result.found
    assert result.code == "x = 1\nprint(x)"
    assert result.notes == []


def test_accepts_py_alias():
    assert extract_code("```py\nx = 1\n```").code == "x = 1"


def test_accepts_untagged_fence_with_note():
    result = extract_code("```\nx = 1\n```")
    assert result.code == "x = 1"
    # An empty fence tag is a valid Python tag, so no note is expected.
    assert result.notes == []


def test_rejects_non_python_tag_with_note():
    result = extract_code("```bash\necho hi\n```")
    assert result.code == "echo hi"
    assert any("tagged `bash`" in note for note in result.notes)


def test_last_block_wins_with_note():
    response = "```python\na = 1\n```\ntext\n```python\nb = 2\n```"
    result = extract_code(response)
    assert result.code == "b = 2"
    assert result.raw_blocks == 2
    assert any("2 code blocks" in note for note in result.notes)


def test_unclosed_fence_is_reported_and_used():
    result = extract_code("```python\nx = 1\ny = 2")
    assert "y = 2" in result.code
    assert any("not closed" in note for note in result.notes)


def test_fence_inside_string_is_not_the_end():
    response = '```python\nmsg = "a ``` b"\nprint(len(msg))\n```'
    result = extract_code(response)
    assert "print(len(msg))" in result.code
    assert any("string literal" in note for note in result.notes)


def test_note_prefix_is_rendered_once():
    result = extract_code("```bash\necho hi\n```")
    observation = result.observation("output")
    assert observation.startswith("[note]")
    assert observation.endswith("output")


def test_no_note_means_unprefixed_observation():
    result = extract_code("```python\nx = 1\n```")
    assert result.observation("output") == "output"


# -- alternative formats ---------------------------------------------------
def test_anthropic_invoke_is_converted():
    response = (
        "<invoke><tool_name>read_file</tool_name>"
        '<parameter name="filepath">a.py</parameter>'
        '<parameter name="start_line">3</parameter>'
        "</invoke>"
    )
    result = extract_code(response)
    assert result.code.startswith("read_file(")
    assert "filepath='a.py'" in result.code
    assert "start_line=3" in result.code
    assert any("converted" in note for note in result.notes)


def test_hermes_tool_call_is_converted():
    response = '<tool_call>{"name": "run_tests", "arguments": {"code": "x = 1"}}</tool_call>'
    result = extract_code(response)
    assert result.code.startswith("run_tests(")
    assert "code='x = 1'" in result.code
    assert any("converted" in note for note in result.notes)


def test_hermes_tool_call_with_stringified_arguments():
    response = '<tool_call>{"name": "edit_file", "arguments": "{\\"filepath\\": \\"a.py\\"}"}</tool_call>'
    result = extract_code(response)
    assert "edit_file(" in result.code
    assert "filepath='a.py'" in result.code


def test_react_action_is_converted():
    response = "Thought: look\nAction: read_file\nAction Input: {\"filepath\": \"a.py\"}\n"
    result = extract_code(response)
    assert result.code.startswith("read_file(")
    assert any("ReAct" in note for note in result.notes)


def test_react_action_with_plain_string_input():
    response = "Action: run_command\nAction Input: ls -la\nObservation: whatever\n"
    result = extract_code(response)
    assert result.code == "run_command('ls -la')"


def test_react_action_without_input():
    response = "Action: get_patch\nObservation: later\n"
    assert extract_code(response).code == "get_patch()"


def test_bare_code_label_is_accepted_with_note():
    response = "Thought: here\nCode:\nx = 41"
    result = extract_code(response)
    assert result.code == "x = 41"
    assert any("bare `Code:`" in note for note in result.notes)


def test_no_code_returns_empty_extraction():
    result = extract_code("Just some prose without any code.")
    assert not result.found
    assert result.code == ""


def test_none_response_is_safe():
    assert extract_code(None).found is False


def test_empty_string_is_safe():
    assert extract_code("").found is False


# -- helpers ---------------------------------------------------------------
def test_looks_like_python():
    assert looks_like_python("x = 1")
    assert looks_like_python("def f(:\n  pass") is False


def test_count_code_blocks():
    response = "```python\na\n```\n```python\nb\n```"
    assert count_code_blocks(response) == 2
    assert count_code_blocks("") == 0


def test_crlf_is_normalised():
    result = extract_code("```python\r\nx = 1\r\n```")
    assert result.code == "x = 1"


def test_tool_name_dashes_are_normalised():
    response = '<tool_call>{"name": "run-tests", "arguments": {}}</tool_call>'
    assert "run_tests()" in extract_code(response).code


def test_multiple_notes_accumulate():
    response = "```bash\necho one\n```\n```python\nx = 1"
    result = extract_code(response)
    assert len(result.notes) >= 2
