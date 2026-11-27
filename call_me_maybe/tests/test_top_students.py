"""Tests "grands élèves" pour la robustesse du décodage contraint.

Ils vérifient la garantie structurelle du programme (le format JSON, le
masquage des logits, le formatage des nombres, les frontières des
sources et les erreurs CLI) sans dépendre du poids du modèle.
"""

import json
import subprocess
import sys
import tempfile
from pathlib import Path
from types import SimpleNamespace

import numpy as np

from src.constraints import (
    ChoiceConstraint,
    NullConstraint,
    NumberConstraint,
    StringConstraint,
)
from src.decoder import FunctionCaller
from src.io_utils import write_results
from src.models import FunctionDefinition
from src.vocab import Vocabulary

ROOT = Path(__file__).resolve().parents[1]


class ScriptedLLM:
    """LLM mock whose logits favor a scripted sequence of token ids."""

    def __init__(self, script: list[int], size: int = 512) -> None:
        self.script = list(script)
        self.pos = 0
        self.size = size

    def encode(self, text: str) -> SimpleNamespace:
        """Return a fake 2-D tensor of character ids (all zeros)."""
        return SimpleNamespace(tolist=lambda: [[0] * len(text)])

    def get_logits_from_input_ids(self, ids: list[int]) -> list[float]:
        """Return logits favoring the next scripted token."""
        if self.pos < len(self.script):
            target = self.script[self.pos]
            self.pos += 1
        else:
            target = self.script[-1] if self.script else 0
        return [10.0 if i == target else -100.0 for i in range(self.size)]


def token_id(vocab: Vocabulary, text: str) -> int:
    """Return the id of a token by its text."""
    for token_id in range(max(vocab.text_to_id.values()) + 1):
        if vocab.text_of(token_id) == text:
            return token_id
    raise KeyError(text)


# -- masquage des logits ------------------------------------------------


def test_logit_mask_restricts_next_token(vocab: Vocabulary) -> None:
    """The decoder picks within the allowed ids (no leakage)."""
    constraint = NumberConstraint(vocab=vocab, follow="}}")
    allowed = {int(i) for i in constraint.allowed_ids("")}
    assert token_id(vocab, "1") in allowed
    assert token_id(vocab, "2") in allowed
    llm = ScriptedLLM([token_id(vocab, "2")])
    caller = FunctionCaller(llm=llm, vocab=vocab, functions=[])
    chosen = caller.next_token([0], np.array(sorted(allowed)))
    assert chosen == token_id(vocab, "2")
    assert chosen in allowed


def test_number_constraint_rejects_invalid_digits(vocab: Vocabulary) -> None:
    """Le nombre généré est toujours un nombre JSON valide."""
    constraint = NumberConstraint(vocab=vocab, follow="}}")
    buffer, done = constraint.advance("1", "2")
    assert not done
    buffer, done = constraint.advance(buffer, "3")
    assert not done
    buffer, done = constraint.advance(buffer, "}}")
    assert done
    assert constraint.to_value(buffer) == 123.0
    allowed = {vocab.text_of(int(i)) for i in constraint.allowed_ids("12")}
    assert "a" not in allowed and "{" not in allowed


# -- format des nombres -------------------------------------------------


def test_number_format_wire_type(vocab: Vocabulary) -> None:
    """Les nombres produits ont un type float compatible JSON."""
    script = [
        token_id(vocab, "1"), token_id(vocab, "2"), token_id(vocab, "."),
        token_id(vocab, "5"), token_id(vocab, "}}"),
    ]
    caller = FunctionCaller(
        llm=ScriptedLLM(script), vocab=vocab, functions=[]
    )
    constraint = NumberConstraint(vocab=vocab, follow="}}")
    text = caller.generate([0], constraint)
    value = constraint.to_value(text)
    assert isinstance(value, float)
    assert value == 12.5


def test_integer_and_boolean_wire_types(vocab: Vocabulary) -> None:
    """integer/boolean ont des types JSON exacts."""
    # integer
    caller = FunctionCaller(
        llm=ScriptedLLM([token_id(vocab, "2"), token_id(vocab, "}}")]),
        vocab=vocab, functions=[],
    )
    int_constraint = NumberConstraint(vocab=vocab, follow="}}", integer=True)
    assert caller.generate([0], int_constraint) == "2"
    assert int_constraint.to_value("2") == 2
    # boolean
    caller = FunctionCaller(
        llm=ScriptedLLM([token_id(vocab, "true")]),
        vocab=vocab, functions=[],
    )
    bool_constraint = ChoiceConstraint(
        vocab=vocab, options=["true", "false"], follow="}}"
    )
    assert caller.generate([0], bool_constraint) == "true"
    assert bool_constraint.to_value("true") is True


# -- JSON strict -------------------------------------------------------


def test_result_is_exact_json_shape() -> None:
    """La sortie contient exactement prompt/name/parameters."""
    from src.models import FunctionCall

    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "results.json"
        results = [FunctionCall(prompt="p", name="fn", parameters={"a": 1.0})]
        write_results(out, results)
        data = json.loads(out.read_text(encoding="utf-8"))
        assert isinstance(data, list)
        assert set(data[0]) == {"prompt", "name", "parameters"}


def test_string_escapes(vocab: Vocabulary) -> None:
    """Les strings générés sont des JSON valides."""
    constraint = StringConstraint(vocab=vocab, follow="}}")
    # Ouverture, corps, fermeture.
    buffer, done = constraint.advance("", '"')
    assert not done
    buffer, done = constraint.advance(buffer, "hi")
    assert not done
    buffer, done = constraint.advance(buffer, '",}}')
    assert done
    assert constraint.to_value(buffer) == "hi"


def test_choice_constraint_choices(vocab: Vocabulary) -> None:
    """Les options sont parmi la liste autorisée."""
    constraint = ChoiceConstraint(vocab=vocab, options=["a", "b"], follow="}}")
    allowed = {vocab.text_of(int(i)) for i in constraint.allowed_ids("")}
    assert allowed <= {"a", "b"}
    assert constraint.to_value("b") == "b"


def test_null_constraint_value(vocab: Vocabulary) -> None:
    """null génère la valeur None."""
    constraint = NullConstraint(vocab=vocab, follow="}}")
    assert constraint.to_value("null") is None


# -- frontières des sources --------------------------------------------


def test_source_boundaries() -> None:
    """Les données d'entrée sont chargées dans data/input."""
    functions_path = ROOT / "data" / "input" / "functions_definition.json"
    tests_path = ROOT / "data" / "input" / "function_calling_tests.json"
    functions = json.loads(functions_path.read_text(encoding="utf-8"))
    tests = json.loads(tests_path.read_text(encoding="utf-8"))
    assert isinstance(functions, list) and functions
    assert isinstance(tests, list) and tests
    for function in functions:
        assert {"name", "description", "parameters"} <= set(function)


def test_functions_are_callable_signature() -> None:
    """Les paramètres des fonctions d'entrée sont des schémas valides."""
    from src.io_utils import load_functions

    functions = load_functions(ROOT / "data" / "input" /
                               "functions_definition.json")
    assert functions
    for function in functions:
        assert isinstance(function, FunctionDefinition)
        for spec in function.parameters.values():
            assert spec.type in ("number", "integer", "string", "boolean",
                                 "null", "array", "object")


# -- erreurs CLI -------------------------------------------------------


def test_cli_bad_files() -> None:
    """Un fichier invalide produit une erreur non triviale."""
    result = subprocess.run(
        [sys.executable, "-m", "src", "--functions_definition", "nope.json",
         "--input", "nope2.json"],
        cwd=ROOT, capture_output=True, text=True,
    )
    assert result.returncode != 0


def test_cli_help() -> None:
    """--help affiche l'usage sans erreur."""
    result = subprocess.run(
        [sys.executable, "-m", "src", "--help"],
        cwd=ROOT, capture_output=True, text=True,
    )
    assert result.returncode == 0
    assert "--model" in result.stdout
    assert "--visualize" in result.stdout


def test_cli_runs_end_to_end() -> None:
    """Le programme s'exécute sur les données fournies (avec mock)."""
    from src.cli import parse_args

    settings = parse_args([
        "--functions_definition",
        str(ROOT / "data" / "input" / "functions_definition.json"),
        "--input",
        str(ROOT / "data" / "input" / "function_calling_tests.json"),
    ])
    assert settings.functions_definition == (
        ROOT / "data" / "input" / "functions_definition.json"
    )
    assert settings.input == (
        ROOT / "data" / "input" / "function_calling_tests.json"
    )
