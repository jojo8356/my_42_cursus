"""Module 5, Exercise 2: Data Pipeline.

Duck typing with Protocol and manual CSV/JSON export plugins.
"""

from typing import Protocol


class ExportPlugin(Protocol):
    """Structural contract for any data export plugin."""

    def process_output(self, data: list) -> str:
        """Return the serialised representation of the data."""
        ...


class CSVExport:
    """Export data as CSV, formatted manually."""

    def process_output(self, data: list) -> str:
        lines = ["name,score"]
        for player in data:
            lines.append(f"{player['name']},{player['score']}")
        return "\n".join(lines)


class JSONExport:
    """Export data as JSON, formatted manually."""

    def process_output(self, data: list) -> str:
        items = []
        for player in data:
            items.append(
                f"{{'name': '{player['name']}', 'score': {player['score']}}}"
            )
        return "[" + ", ".join(items) + "]"


class DataStream:
    """Run registered export plugins over a dataset."""

    def __init__(self) -> None:
        self.plugins: list[ExportPlugin] = []

    def register_plugin(self, plugin: ExportPlugin) -> None:
        """Attach an export plugin."""
        self.plugins.append(plugin)

    def output_pipeline(self, data: list) -> None:
        """Run every plugin and print its output."""
        for plugin in self.plugins:
            label = "CSV" if plugin.__class__.__name__ == "CSVExport" else "JSON"
            print(f"{label} Output:")
            print(plugin.process_output(data))
        print("== DataStream statistics ==")


def main() -> None:
    stream = DataStream()
    stream.register_plugin(CSVExport())
    stream.register_plugin(JSONExport())
    stream.output_pipeline(
        [{"name": "alice", "score": 90}, {"name": "bob", "score": 70}]
    )


if __name__ == "__main__":
    main()
