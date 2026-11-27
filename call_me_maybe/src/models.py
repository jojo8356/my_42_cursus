"""Pydantic models for the input files and the output function calls.

All classes use ``pydantic`` for validation, as the subject requires. The
models describe the function definitions, the test prompts and the final
function-call results, and enforce the supported argument types.
"""

from __future__ import annotations

from pydantic import BaseModel, ConfigDict, field_validator, model_validator

# Argument types the constrained decoder can generate. The first four are the
# subject's core types; ``null``, ``array`` and ``object`` are bonus types.
SUPPORTED_TYPES: tuple[str, ...] = (
    "number",
    "integer",
    "string",
    "boolean",
    "null",
    "array",
    "object",
)

# A generated argument value, narrowed to the supported JSON types.
ArgumentValue = bool | int | float | str | None | list | dict


class ParameterSpec(BaseModel):
    """Type description of one function parameter (or return value).

    Attributes:
        type: JSON type name, one of ``SUPPORTED_TYPES``.
        items: For ``array`` types, the schema of each element (bonus).
        properties: For ``object`` types, the schema of each field (bonus).
    """

    model_config = ConfigDict(extra="ignore")

    type: str
    items: ParameterSpec | None = None
    properties: dict[str, ParameterSpec] | None = None

    @field_validator("type")
    @classmethod
    def check_type(cls, value: str) -> str:
        """Reject types the decoder does not know how to generate.

        Args:
            value: The raw type name read from the file.

        Returns:
            The type name, unchanged.

        Raises:
            ValueError: If the type is not supported.
        """
        if value not in SUPPORTED_TYPES:
            raise ValueError(
                f"unsupported type '{value}' "
                f"(supported: {', '.join(SUPPORTED_TYPES)})"
            )
        return value

    @model_validator(mode="after")
    def check_nested_schema(self) -> "ParameterSpec":
        """Validate the nested schema of ``array``/``object`` types.

        Returns:
            The model itself.

        Raises:
            ValueError: If a nested schema is missing or malformed.
        """
        if self.type == "array":
            if self.items is None:
                raise ValueError("array type requires an 'items' schema")
        if self.type == "object":
            if self.properties is None:
                raise ValueError("object type requires a 'properties' schema")
        return self


class FunctionDefinition(BaseModel):
    """One callable function, as described in functions_definition.json.

    Attributes:
        name: Function name, e.g. ``fn_add_numbers``.
        description: Natural-language description shown to the model.
        parameters: Ordered mapping from argument name to its type.
        returns: Optional return type (only shown to the model).
    """

    model_config = ConfigDict(extra="ignore")

    name: str
    description: str = ""
    parameters: dict[str, ParameterSpec]
    returns: ParameterSpec | None = None

    @field_validator("name")
    @classmethod
    def check_name(cls, value: str) -> str:
        """Require a non-empty function name without quotes or backslashes.

        Args:
            value: The raw function name.

        Returns:
            The function name, unchanged.

        Raises:
            ValueError: If the name is empty or contains forbidden chars.
        """
        if not value.strip():
            raise ValueError("function name must not be empty")
        if '"' in value or "\\" in value:
            raise ValueError("function name must not contain '\"' or '\\'")
        return value

    def check_arguments(self, arguments: dict[str, ArgumentValue]) -> None:
        """Check that arguments match this definition exactly.

        Args:
            arguments: Argument values produced by the decoder.

        Raises:
            ValueError: If a key is missing or extra, or a type is wrong.
        """
        if set(arguments) != set(self.parameters):
            raise ValueError(
                f"arguments {sorted(arguments)} do not match "
                f"parameters {sorted(self.parameters)} of {self.name}"
            )
        for key, spec in self.parameters.items():
            value = arguments[key]
            # bool is a subclass of int in Python, so test it first.
            if isinstance(value, bool):
                valid = spec.type == "boolean"
            elif isinstance(value, int):
                valid = spec.type == "integer"
            elif isinstance(value, float):
                valid = spec.type == "number"
            elif value is None:
                valid = spec.type == "null"
            elif isinstance(value, list):
                valid = spec.type == "array"
            elif isinstance(value, dict):
                valid = spec.type == "object"
            else:
                valid = spec.type == "string"
            if not valid:
                raise ValueError(
                    f"argument '{key}' of {self.name} should be "
                    f"{spec.type}, got {value!r}"
                )


class PromptItem(BaseModel):
    """One test prompt from function_calling_tests.json.

    Attributes:
        prompt: The natural-language request.
    """

    model_config = ConfigDict(extra="ignore")

    prompt: str


class FunctionCall(BaseModel):
    """One result object written to the output file.

    Attributes:
        prompt: The original request.
        name: Name of the chosen function.
        parameters: Argument values with the types from the definition.
    """

    model_config = ConfigDict(extra="forbid", strict=True)

    prompt: str
    name: str
    parameters: dict[str, ArgumentValue]
