"""Module 5, Exercise 1: Data Stream.

A polymorphic orchestrator that routes data to registered processors.
"""

from ex0.data_processor import DataProcessor, NumericProcessor


class DataStream:
    """Route a stream of elements to a registered processor."""

    def __init__(self) -> None:
        self.processors: list[DataProcessor] = []
        self.stats: dict[str, dict[str, int]] = {}

    def register_processor(self, processor: DataProcessor) -> None:
        """Attach a processor and initialise its statistics counters."""
        self.processors.append(processor)
        name = type(processor).__name__
        self.stats[name] = {"total": 0, "remaining": 0}

    def process_stream(self, data: list) -> None:
        """Process each element, reporting errors without crashing."""
        if not self.processors:
            print("No processor found, no data")
            return

        processor = self.processors[0]
        name = type(processor).__name__
        for element in data:
            try:
                if not processor.validate([element]):
                    raise ValueError("invalid element")
                processor.ingest([element])
                self.stats[name]["total"] += 1
                self.stats[name]["remaining"] += 1
            except Exception:
                print(f"DataStream error - Can't process element in stream: {element}")

    def print_processors_stats(self) -> None:
        """Print per-processor statistics."""
        print("== DataStream statistics ==")
        for name, stat in self.stats.items():
            print(
                f"{name} Processor: total {stat['total']} items processed, "
                f"remaining {stat['remaining']} on processor"
            )


def main() -> None:
    stream = DataStream()
    stream.process_stream([1, 2, 3])

    stream.register_processor(NumericProcessor())
    stream.process_stream([1, "bad", 2])
    stream.print_processors_stats()


if __name__ == "__main__":
    main()
