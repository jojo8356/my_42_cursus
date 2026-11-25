"""Module 5, Exercise 0: Data Processor.

An abstract base class with three concrete implementations.
"""

from abc import ABC, abstractmethod


class DataProcessor(ABC):
    """Abstract base defining the processing contract."""

    @abstractmethod
    def validate(self, data: list) -> bool:
        """Return whether the input is valid for this processor."""

    @abstractmethod
    def ingest(self, data: list) -> list:
        """Transform raw input into processed values."""

    def output(self, data: list) -> None:
        """Print each processed value on its own line."""
        for index, value in enumerate(data):
            print(f"Output {index}: {value}")


class NumericProcessor(DataProcessor):
    """Process numeric data."""

    def validate(self, data: list) -> bool:
        print("Trying to validate input")
        return all(isinstance(value, (int, float)) for value in data)

    def ingest(self, data: list) -> list:
        values = [value for value in data if isinstance(value, (int, float))]
        print(f"Extracting {len(values)} values")
        return values

    def output(self, data: list) -> None:
        for index, value in enumerate(data):
            print(f"Numeric value {index}: {value}")


class TextProcessor(DataProcessor):
    """Process text data."""

    def validate(self, data: list) -> bool:
        print("Trying to validate input")
        return all(isinstance(value, str) for value in data)

    def ingest(self, data: list) -> list:
        print(f"Extracting {len(data)} values")
        return [value.upper() for value in data]

    def output(self, data: list) -> None:
        for index, value in enumerate(data):
            print(f"Text value {index}: {value}")


class LogProcessor(DataProcessor):
    """Process log entries."""

    def validate(self, data: list) -> bool:
        print("Trying to validate input")
        return all(isinstance(value, str) for value in data)

    def ingest(self, data: list) -> list:
        print(f"Extracting {len(data)} values")
        return list(data)

    def output(self, data: list) -> None:
        for index, value in enumerate(data):
            print(f"Log entry {index}: {value}")


def main() -> None:
    numeric = NumericProcessor()
    print("Testing Numeric Processor")
    if numeric.validate([1, 2, 3]):
        numeric.output(numeric.ingest([1, 2, 3]))

    text = TextProcessor()
    print("Testing Text Processor")
    if text.validate(["a", "b"]):
        text.output(text.ingest(["a", "b"]))

    log = LogProcessor()
    print("Testing Log Processor")
    if log.validate(["boot", "ready"]):
        log.output(log.ingest(["boot", "ready"]))


if __name__ == "__main__":
    main()
