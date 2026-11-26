"""Interactive terminal menu for A-Maze-ing."""

from __future__ import annotations

from collections.abc import Callable

from .generator import MazeGenerator
from .renderer import PALETTE, render

MENU = (
    "=== A-Maze-ing ===\n"
    "[1] Re-generate a new maze\n"
    "[2] Show / Hide the shortest path\n"
    "[3] Rotate the wall colours\n"
    "[4] Quit\n"
    "Enter a choice: "
)


def interactive_session(
    generator: MazeGenerator,
    regenerate: Callable[[], MazeGenerator],
) -> None:
    """Run the interactive menu.

    `generator` is the current MazeGenerator, `regenerate` a callable that
    generates a new maze, writes the output file and returns the generator.
    """
    show_path = False
    palette_index = 0
    while True:
        assert generator.maze is not None
        maze = generator.maze
        path = generator.path
        palette = PALETTE[palette_index % len(PALETTE)]
        print(render(maze, show_path=show_path, path=path, palette=palette))
        print(MENU, end="", flush=True)
        try:
            choice = input().strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break
        if choice in ("1", "regen", "regenerate"):
            generator = regenerate()
            show_path = False
        elif choice in ("2", "path"):
            show_path = not show_path
        elif choice in ("3", "color", "colour"):
            palette_index += 1
        elif choice in ("4", "q", "quit", "exit"):
            break
        else:
            print("Invalid choice, try again.")
