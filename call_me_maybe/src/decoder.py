"""Constrained decoding: turn one prompt into one valid function call.

The answer is always built in the same order, and only the variable parts
are asked to the model::

    {"name": "<NAME>", "arguments": {"a":<VALUE>, "b":<VALUE>}}

(``arguments`` is the key Qwen was trained with; the output file uses
``parameters``.) Everything outside ``<...>`` is fixed text that we add
ourselves, without calling the model. Each ``<...>`` is generated token
by token under a constraint that masks every token which would break the
schema.
"""

import json
import sys
from typing import Any

import numpy as np
from pydantic import BaseModel, ConfigDict

from src.constraints import (
    ChoiceConstraint,
    Constraint,
    NullConstraint,
    NumberConstraint,
    StringConstraint,
)
from src.models import (
    ArgumentValue,
    FunctionCall,
    FunctionDefinition,
    ParameterSpec,
)
from src.prompting import ARGUMENTS_KEY, build_prompt
from src.vocab import Vocabulary


class FunctionCaller(BaseModel):
    """Generate function calls with a model and the constraints.

    Attributes:
        llm: The ``Small_LLM_Model`` from llm_sdk (public methods only).
        vocab: Decoded vocabulary of the same model.
        functions: The functions the model may call.
        model_calls: Number of forward passes done so far (statistics).
        visualize: Print each generated token to stderr (bonus).
    """

    model_config = ConfigDict(arbitrary_types_allowed=True)

    llm: Any
    vocab: Vocabulary
    functions: list[FunctionDefinition]
    model_calls: int = 0
    visualize: bool = False

    def encode(self, text: str) -> list[int]:
        """Tokenize text with the model's tokenizer.

        Args:
            text: Text to tokenize.

        Returns:
            The token ids as plain Python ints.
        """
        rows = self.llm.encode(text).tolist()
        return [int(token_id) for token_id in rows[0]]

    def next_token(self, ids: list[int], allowed: np.ndarray) -> int:
        """Choose the best allowed token after ``ids``.

        If only one token is allowed the model is not called at all.

        Args:
            ids: Full context (prompt + answer so far).
            allowed: Token ids allowed by the constraint.

        Returns:
            The chosen token id.
        """
        if len(allowed) == 1:
            return int(allowed[0])
        logits = np.asarray(
            self.llm.get_logits_from_input_ids(ids), dtype=np.float64
        )
        self.model_calls += 1
        masked = np.full(logits.shape, -np.inf)
        masked[allowed] = logits[allowed]
        return int(np.argmax(masked))

    def generate(self, ids: list[int], constraint: Constraint) -> str:
        """Generate one value token by token under a constraint.

        Args:
            ids: Context ending right where the value starts.
            constraint: Rules for this value.

        Returns:
            The complete value text.
        """
        context = list(ids)
        buffer = ""
        for _ in range(constraint.max_tokens):
            allowed = constraint.allowed_ids(buffer)
            token_id = self.next_token(context, allowed)
            if self.visualize:
                print(
                    f"  {'value':20} | {len(allowed):6} allowed | "
                    f"{token_id:6} | {self.vocab.text_of(token_id)!r}",
                    file=sys.stderr,
                )
            buffer, done = constraint.advance(
                buffer, self.vocab.text_of(token_id)
            )
            if done:
                return buffer
            context.append(token_id)
        return constraint.force_close(buffer)

    def constraint_for(
        self, spec: ParameterSpec, follow: str,
        follow_any: list[str] | None = None,
    ) -> Constraint:
        """Create the constraint matching a parameter type.

        Args:
            spec: Parameter type.
            follow: Fixed text written after the value.
            follow_any: Optional alternative follow texts (array elements).

        Returns:
            The constraint to use.
        """
        if spec.type == "string":
            return StringConstraint(
                vocab=self.vocab, follow=follow, follow_any=follow_any,
            )
        if spec.type == "boolean":
            return ChoiceConstraint(
                vocab=self.vocab,
                follow=follow,
                follow_any=follow_any,
                options=[" true", " false", "true", "false"],
            )
        if spec.type == "null":
            return NullConstraint(
                vocab=self.vocab, follow=follow, follow_any=follow_any,
            )
        return NumberConstraint(
            vocab=self.vocab, follow=follow, follow_any=follow_any,
            integer=spec.type == "integer",
        )

    def generate_with_closing(
        self, ids: list[int], constraint: Constraint,
    ) -> tuple[str, str]:
        """Generate one value and return the closing token text.

        Args:
            ids: Context ending right where the value starts.
            constraint: Rules for this value.

        Returns:
            The value text and the text of the token that completed it.
        """
        context = list(ids)
        buffer = ""
        closing = ""
        for _ in range(constraint.max_tokens):
            allowed = constraint.allowed_ids(buffer)
            token_id = self.next_token(context, allowed)
            if self.visualize:
                print(
                    f"  {'value':20} | {len(allowed):6} allowed | "
                    f"{token_id:6} | {self.vocab.text_of(token_id)!r}",
                    file=sys.stderr,
                )
            token_text = self.vocab.text_of(token_id)
            buffer, done = constraint.advance(buffer, token_text)
            if done:
                closing = token_text
                return buffer, closing
            context.append(token_id)
        return constraint.force_close(buffer), ""

    def _continue_value(
        self, context: list[int], constraint: Constraint, buffer: str,
    ) -> str:
        """Continue generating a value from a non-empty buffer.

        Args:
            context: Context ending right where the value started.
            constraint: Rules for this value.
            buffer: Text already generated for the value.

        Returns:
            The complete value text.
        """
        for _ in range(constraint.max_tokens):
            allowed = constraint.allowed_ids(buffer)
            token_id = self.next_token(context, allowed)
            if self.visualize:
                print(
                    f"  {'value':20} | {len(allowed):6} allowed | "
                    f"{token_id:6} | {self.vocab.text_of(token_id)!r}",
                    file=sys.stderr,
                )
            token_text = self.vocab.text_of(token_id)
            buffer, done = constraint.advance(buffer, token_text)
            if done:
                return buffer
            context.append(token_id)
        return constraint.force_close(buffer)

    def generate_array(
        self, ids: list[int], items: ParameterSpec,
    ) -> tuple[str, list[ArgumentValue]]:
        """Generate a JSON array of values of the given element schema.

        The brackets are fixed text; each element is generated by the
        model under its constraint. Before each element the model may
        also choose to close the array (``]``), which allows empty
        arrays.

        Args:
            ids: Context ending right where the array starts.
            items: Schema of each element.

        Returns:
            The array text (without the outer brackets) and the values.
        """
        context = list(ids)
        context.extend(self.encode("["))
        elements: list[ArgumentValue] = []
        parts: list[str] = []
        while True:
            constraint = self.constraint_for(
                items, follow=",", follow_any=["]"],
            )
            # The model may close the array or start a new element.
            close_ids = self.vocab.ids_for_prefixes("]")
            value_ids = [int(i) for i in constraint.allowed_ids("")]
            allowed = sorted(set(value_ids) | set(close_ids))
            token_id = self.next_token(context, np.array(allowed))
            if self.vocab.text_of(token_id).startswith("]"):
                break
            # The chosen token begins the value; finish it.
            token_text = self.vocab.text_of(token_id)
            buffer, done = constraint.advance("", token_text)
            context.append(token_id)
            if not done:
                buffer = self._continue_value(context, constraint, buffer)
            elements.append(constraint.to_value(buffer))
            parts.append(buffer)
            context.extend(self.encode(", "))
        return ", ".join(parts), elements

    def generate_object(
        self, ids: list[int], properties: dict[str, ParameterSpec],
    ) -> tuple[str, dict[str, ArgumentValue]]:
        """Generate a JSON object with the given field schemas.

        The braces, keys and colons are fixed text; only the values are
        generated by the model under their constraints.

        Args:
            ids: Context ending right where the object starts.
            properties: Schema of each field (in order).

        Returns:
            The object text (without the outer braces) and the values.
        """
        context = list(ids)
        context.extend(self.encode("{"))
        values: dict[str, ArgumentValue] = {}
        text = ""
        items = list(properties.items())
        for index, (key, spec) in enumerate(items):
            is_last = index == len(items) - 1
            follow = "}" if is_last else ", "
            key_text = json.dumps(key) + ":"
            context.extend(self.encode(key_text))
            constraint = self.constraint_for(spec, follow)
            value_text = self.generate(context, constraint)
            values[key] = constraint.to_value(value_text)
            piece = key_text + value_text
            text = piece if index == 0 else text + ", " + piece
            if is_last:
                break
            context.extend(self.encode(", "))
        return text, values

    def call(self, request: str) -> FunctionCall:
        """Produce a validated function call for one request.

        Args:
            request: Natural-language prompt.

        Returns:
            The function call, checked against its definition.
        """
        prompt_ids = self.encode(build_prompt(self.functions, request))
        arguments_start = f', "{ARGUMENTS_KEY}": {{'

        # 1. Function name: one of the known names, followed by a quote.
        answer = '{"name": "'
        name_constraint = ChoiceConstraint(
            vocab=self.vocab,
            options=[function.name + '"' for function in self.functions],
            follow=arguments_start,
            max_tokens=max(len(f.name) for f in self.functions) + 20,
        )
        name_text = self.generate(
            prompt_ids + self.encode(answer), name_constraint
        )
        function = next(f for f in self.functions if f.name + '"' == name_text)
        answer += name_text + arguments_start

        # 2. Arguments, in the order of the definition. The fixed text
        # stops right after the colon: the model writes the value with its
        # own leading space (" -2", " true", ' "hello'), as it was trained.
        arguments: dict[str, ArgumentValue] = {}
        parameters = list(function.parameters.items())
        for index, (key, spec) in enumerate(parameters):
            is_last = index == len(parameters) - 1
            follow = "}}" if is_last else ", "
            answer += json.dumps(key) + ":"
            context = prompt_ids + self.encode(answer)
            if spec.type == "array" and spec.items is not None:
                value_text, elements = self.generate_array(context, spec.items)
                arguments[key] = elements
                answer += "[" + value_text + "]" + follow
            elif spec.type == "object" and spec.properties is not None:
                value_text, values = self.generate_object(
                    context, spec.properties
                )
                arguments[key] = values
                answer += "{" + value_text + "}" + follow
            else:
                constraint = self.constraint_for(spec, follow)
                value_text = self.generate(context, constraint)
                arguments[key] = constraint.to_value(value_text)
                closing_quote = '"' if spec.type == "string" else ""
                answer += value_text + closing_quote + follow
        if not parameters:
            answer += "}}"

        call = FunctionCall(
            prompt=request, name=function.name, parameters=arguments
        )
        function.check_arguments(call.parameters)
        return call
