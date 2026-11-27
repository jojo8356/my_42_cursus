"""Tests for the bonus features: null/array/object types, visualize,
multiple models and per-prompt timeout."""

import json
from types import SimpleNamespace

import pytest

from src.constraints import NullConstraint
from src.decoder import FunctionCaller
from src.models import FunctionDefinition, ParameterSpec
from src.prompting import function_schema
from src.vocab import Vocabulary, bytes_to_unicode


def make_vocab(tokens: list[str]) -> Vocabulary:
    """Build a Vocabulary from a list of token texts.

    Args:
        tokens: Token texts; the id is the position in the list.

    Returns:
        The decoded vocabulary.
    """
    table = bytes_to_unicode()
    token_map = {
        "".join(table[byte] for byte in text.encode("utf-8")): index
        for index, text in enumerate(tokens)
    }
    return Vocabulary.from_token_map(token_map)


# Tokens needed for array/object generation.
BONUS_TOKENS = [
    "[", "]", ",", ", ", "{", "}", ":", "0", "1", "2", "3", "true",
    "false", "null", '"', '"}}', "}}", "}", ", ", " ", " -", "-", ".",
    "fn", "_add", '"',
]


class MockLLM:
    """LLM mock returning logits that favor a scripted token sequence."""

    def __init__(self, script: list[int], size: int = 512) -> None:
        self.script = list(script)
        self.pos = 0
        self.size = size

    def encode(self, text: str) -> SimpleNamespace:
        """Return a fake 2-D tensor of token ids (all zeros)."""
        return SimpleNamespace(tolist=lambda: [[0] * len(text)])

    def get_logits_from_input_ids(self, ids: list[int]) -> list[float]:
        """Return logits favoring the next scripted token."""
        if self.pos < len(self.script):
            target = self.script[self.pos]
            self.pos += 1
        else:
            target = self.script[-1] if self.script else 0
        logits = [-100.0] * self.size
        logits[target] = 10.0
        return logits


# -- null constraint -----------------------------------------------------


def test_null_constraint() -> None:
    """The null constraint generates the literal ``null``."""
    vocab = make_vocab(["n", "nu", "null", "}}", ","])
    null = NullConstraint(vocab=vocab, follow="}}")
    allowed = {vocab.text_of(int(i)) for i in null.allowed_ids("")}
    assert "n" in allowed
    buffer, done = null.advance("", "null")
    assert done and null.to_value(buffer) is None
    assert null.force_close("") == "null"


# -- models: bonus types ------------------------------------------------


def test_parameter_spec_bonus_types() -> None:
    """null/array/object schemas validate correctly."""
    assert ParameterSpec(type="null").type == "null"
    array = ParameterSpec(type="array", items=ParameterSpec(type="number"))
    assert array.items is not None
    assert array.items.type == "number"
    obj = ParameterSpec(
        type="object",
        properties={"a": ParameterSpec(type="number")},
    )
    assert obj.properties is not None
    assert obj.properties["a"].type == "number"
    # array without items is rejected.
    with pytest.raises(ValueError, match="items"):
        ParameterSpec(type="array")
    # object without properties is rejected.
    with pytest.raises(ValueError, match="properties"):
        ParameterSpec(type="object")


def test_check_arguments_bonus_types() -> None:
    """check_arguments accepts null/array/object values."""
    definition = FunctionDefinition.model_validate({
        "name": "fn", "description": "",
        "parameters": {
            "n": {"type": "null"},
            "a": {"type": "array", "items": {"type": "number"}},
            "o": {"type": "object", "properties": {"x": {"type": "number"}}},
        },
    })
    definition.check_arguments({"n": None, "a": [1.0, 2.0], "o": {"x": 1.0}})
    with pytest.raises(ValueError, match="should be null"):
        definition.check_arguments({"n": 1, "a": [], "o": {}})
    with pytest.raises(ValueError, match="should be array"):
        definition.check_arguments({"n": None, "a": "x", "o": {}})
    with pytest.raises(ValueError, match="should be object"):
        definition.check_arguments({"n": None, "a": [], "o": "x"})


# -- prompting: nested schemas ------------------------------------------


def test_function_schema_nested() -> None:
    """Nested schemas are rendered compactly in the prompt."""
    function = FunctionDefinition.model_validate({
        "name": "fn", "description": "d",
        "parameters": {
            "a": {"type": "array", "items": {"type": "number"}},
            "o": {"type": "object", "properties": {"x": {"type": "string"}}},
        },
    })
    schema = json.loads(function_schema(function))
    assert schema["parameters"]["a"] == "array<number>"
    assert schema["parameters"]["o"] == "object{x:string}"


# -- decoder: array / object generation --------------------------------


def test_generate_array_numbers() -> None:
    """An array of numbers is generated with the right values."""
    vocab = make_vocab(BONUS_TOKENS)
    ids = {token: index for index, token in enumerate(BONUS_TOKENS)}
    script = [ids["1"], ids[","], ids["2"], ids["]"]]
    llm = MockLLM(script)
    caller = FunctionCaller(
        llm=llm, vocab=vocab, functions=[],
    )
    items = ParameterSpec(type="number", items=None, properties=None)
    text, elements = caller.generate_array([], items)
    assert elements == [1.0, 2.0]
    assert text == "1, 2"


def test_generate_object_fields() -> None:
    """An object with mixed fields is generated correctly."""
    vocab = make_vocab(BONUS_TOKENS)
    ids = {token: index for index, token in enumerate(BONUS_TOKENS)}
    # Script: value for "a" (1), closing "}", then value for "b" ("2")...
    # Actually object generation writes keys itself; the model only
    # generates values. Values: 1 then ", " then 2 then "}".
    script = [ids["1"], ids[", "], ids["2"], ids["}"]]
    llm = MockLLM(script)
    caller = FunctionCaller(llm=llm, vocab=vocab, functions=[])
    properties = {
        "a": ParameterSpec(type="number"),
        "b": ParameterSpec(type="number"),
    }
    text, values = caller.generate_object([], properties)
    assert values == {"a": 1.0, "b": 2.0}
    assert "a" in text and "b" in text


def test_generate_array_empty() -> None:
    """An empty array is produced when the model closes immediately."""
    vocab = make_vocab(BONUS_TOKENS)
    ids = {token: index for index, token in enumerate(BONUS_TOKENS)}
    script = [ids["]"]]
    llm = MockLLM(script)
    caller = FunctionCaller(llm=llm, vocab=vocab, functions=[])
    items = ParameterSpec(type="number")
    text, elements = caller.generate_array([], items)
    assert elements == []
    assert text == ""


# -- visualize ----------------------------------------------------------


def test_visualize_prints_tokens(
    capsys: pytest.CaptureFixture[str],
) -> None:
    """The visualize option prints each generated token to stderr."""
    vocab = make_vocab(BONUS_TOKENS)
    ids = {token: index for index, token in enumerate(BONUS_TOKENS)}
    script = [ids["1"], ids["]"]]
    llm = MockLLM(script)
    caller = FunctionCaller(llm=llm, vocab=vocab, functions=[], visualize=True)
    items = ParameterSpec(type="number")
    caller.generate_array([], items)
    captured = capsys.readouterr()
    assert "allowed" in captured.err
    assert "1" in captured.err


# -- CLI: bonus options -------------------------------------------------


def test_cli_bonus_options() -> None:
    """The CLI parses --model, --visualize and --timeout."""
    from src.cli import parse_args

    settings = parse_args(
        ["--model", "Qwen/Qwen3-1.7B", "--visualize", "--timeout", "30"]
    )
    assert settings.model == "Qwen/Qwen3-1.7B"
    assert settings.visualize is True
    assert settings.timeout == 30.0
    # Defaults.
    defaults = parse_args([])
    assert defaults.model == "Qwen/Qwen3-0.6B"
    assert defaults.visualize is False
    assert defaults.timeout == 60.0
