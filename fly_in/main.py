"""Fly-In entry point: parse arguments, run the simulation, print the log.

Usage::

    python3 main.py [MAP] [--plain] [--no-menu]

With no ``MAP`` argument the program presents an interactive menu of the
supplied maps (and accepts a custom path).  Stdout carries only the
movement log; menus, prompts and errors go to stderr.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from src.parser import MapError, MapParser
from src.planner import ReservationRouter, ScheduleError
from src.renderer import MovementRenderer
from src.simulation import ReplayChecker, ValidationError

# The supplied maps, grouped, resolved relative to the repository root.
_MAP_ROOT = Path(__file__).resolve().parent / "maps"
_MENU_MAPS = [
    ("easy", "01_linear_path.txt"),
    ("easy", "02_simple_fork.txt"),
    ("easy", "03_basic_capacity.txt"),
    ("medium", "01_dead_end_trap.txt"),
    ("medium", "02_circular_loop.txt"),
    ("medium", "03_priority_puzzle.txt"),
    ("hard", "01_maze_nightmare.txt"),
    ("hard", "02_capacity_hell.txt"),
    ("hard", "03_ultimate_challenge.txt"),
    ("challenger", "01_the_impossible_dream.txt"),
]


def build_arg_parser() -> argparse.ArgumentParser:
    """Define the command-line arguments."""
    parser = argparse.ArgumentParser(
        prog="fly-in",
        description="Multi-drone routing simulation (42 curriculum).",
    )
    parser.add_argument(
        "map",
        nargs="?",
        help="path to a map file (optional; omit to open the menu)",
    )
    parser.add_argument(
        "--plain",
        action="store_true",
        help="disable colored terminal output",
    )
    parser.add_argument(
        "--no-menu",
        action="store_true",
        help="do not open the menu when no map path is supplied",
    )
    return parser


def list_maps() -> list[Path]:
    """Return the absolute paths of the supplied maps, in menu order."""
    return [_MAP_ROOT / category / filename for category, filename in _MENU_MAPS]


def choose_map() -> Path:
    """Present an interactive menu and return the selected map path."""
    maps = list_maps()
    print("Available maps:", file=sys.stderr)
    for index, path in enumerate(maps, 1):
        print(f"  {index}: {path}", file=sys.stderr)
    print("  p: custom path    q: quit", file=sys.stderr)

    while True:
        try:
            answer = input("Select a map: ").strip()
        except EOFError:
            raise SystemExit(0)
        if answer in ("q", "quit", ""):
            raise SystemExit(0)
        if answer == "p":
            raw = input("Custom path: ").strip()
            candidate = Path(raw)
            if candidate.is_file():
                return candidate
            print(f"File not found: {raw}", file=sys.stderr)
            continue
        if answer.isdigit() and 1 <= int(answer) <= len(maps):
            return maps[int(answer) - 1]
        print("Invalid selection.", file=sys.stderr)


def run(map_path: Path, color: bool) -> int:
    """Parse, plan, validate and render a single map. Return a status code."""
    try:
        network = MapParser().parse(map_path)
    except (MapError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    try:
        routes = ReservationRouter(network).plan()
        ReplayChecker(network).check(routes)
    except (ScheduleError, ValidationError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 3

    MovementRenderer(network, color=color).print_run(routes)
    return 0


def main(argv: list[str] | None = None) -> int:
    """Coordinate argument parsing and map selection."""
    args = build_arg_parser().parse_args(argv)
    color = not args.plain

    if args.map:
        return run(Path(args.map), color)
    if args.no_menu:
        print("error: no map provided", file=sys.stderr)
        return 2
    return run(choose_map(), color)


if __name__ == "__main__":
    raise SystemExit(main())
