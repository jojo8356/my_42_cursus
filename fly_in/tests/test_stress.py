"""Stress and fuzzing tests for enterprise-grade robustness.

Generates many seeded random networks, solves and validates each with the
independent replay checker, and fuzzes the parser with malformed inputs to
ensure no unexpected exceptions leak out.
"""
from __future__ import annotations

import random
import tempfile
from pathlib import Path

import pytest

from src.parser import MapError, MapParser
from src.planner import ReservationRouter
from src.simulation import ReplayChecker

KINDS = ["normal", "priority", "restricted"]


def _random_map(rng: random.Random, zone_count: int = 6) -> str:
    """Build a random, always-valid map string (no duplicate links)."""
    names = [f"z{i}" for i in range(zone_count)]
    lines = [f"nb_drones: {rng.randint(1, 6)}"]
    start = "start"
    end = "end"
    lines.append(f"start_hub: {start} 0 0")
    lines.append(f"end_hub: {end} {zone_count} 0")
    for name in names:
        kind = rng.choice(KINDS)
        cap = rng.choice([1, 1, 2, 3])
        lines.append(f"hub: {name} {rng.randint(-3, 9)} {rng.randint(-3, 9)} "
                     f"[zone={kind} max_drones={cap}]")

    used: set[tuple[str, str]] = set()

    def _add(a: str, b: str, cap: int = 1) -> None:
        key = (min(a, b), max(a, b))
        if key in used:
            return
        used.add(key)
        if cap == 1:
            lines.append(f"connection: {a}-{b}")
        else:
            lines.append(f"connection: {a}-{b} [max_link_capacity={cap}]")

    # Guaranteed backbone so the end is always reachable.
    for i in range(zone_count - 1):
        _add(names[i], names[i + 1])
    _add(names[0], start)
    _add(names[-1], end)
    # Add a few extra random links.
    for _ in range(rng.randint(0, 4)):
        a, b = rng.sample(names, 2)
        _add(a, b, rng.choice([1, 2]))
    return "\n".join(lines) + "\n"


def _parse(text: str) -> Path:
    """Write ``text`` to a temp file and return its path."""
    handle = tempfile.NamedTemporaryFile(
        "w", suffix=".txt", delete=False, encoding="utf-8"
    )
    handle.write(text)
    handle.close()
    return Path(handle.name)


@pytest.mark.parametrize("seed", range(40))
def test_random_network_solves_and_validates(seed: int) -> None:
    """A seeded random network is solved and passes the replay checker."""
    rng = random.Random(seed)
    text = _random_map(rng)
    network = MapParser().parse(_parse(text))
    routes = ReservationRouter(network).plan()
    ReplayChecker(network).check(routes)
    assert len(routes) == network.drones


def test_large_fleet_performance() -> None:
    """A wide backbone with many drones is solved quickly and validly."""
    text = "nb_drones: 200\nstart_hub: s 0 0\nend_hub: e 10 0\n"
    for i in range(9):
        text += f"hub: n{i} {i + 1} 0 [max_drones=200]\n"
    for i in range(9):
        prev = "s" if i == 0 else f"n{i - 1}"
        text += f"connection: {prev}-n{i} [max_link_capacity=200]\n"
    text += "connection: n8-e [max_link_capacity=200]\n"
    network = MapParser().parse(_parse(text))
    routes = ReservationRouter(network).plan()
    ReplayChecker(network).check(routes)
    assert len(routes) == 200


@pytest.mark.parametrize("seed", range(100))
def test_parser_fuzz_no_unexpected_exceptions(seed: int) -> None:
    """Malformed random inputs raise MapError, never an unexpected crash."""
    rng = random.Random(seed)
    tokens = [
        "nb_drones:", "1", "2", "start_hub:", "end_hub:", "hub:",
        "connection:", "s", "e", "a", "-", "[", "]", "=", "zone=normal",
        "max_drones=2", "0", "-1", "#comment", "x y z", "[color=red]",
    ]
    text = " ".join(rng.choice(tokens) for _ in range(rng.randint(1, 20))) + "\n"
    try:
        MapParser().parse(_parse(text))
    except MapError:
        pass
    except (ValueError, IndexError, KeyError) as error:
        pytest.fail(f"seed {seed}: unexpected {type(error).__name__}: {error}")


def test_self_loop_and_duplicate_free_fuzz() -> None:
    """Random maps with duplicate/self links still parse or reject cleanly."""
    rng = random.Random(1234)
    for _ in range(25):
        text = _random_map(rng, zone_count=4)
        # Inject a duplicate connection and a self-connection.
        text += "connection: z0-z1\n"
        text += "connection: z2-z2\n"
        try:
            network = MapParser().parse(_parse(text))
        except MapError:
            continue
        routes = ReservationRouter(network).plan()
        ReplayChecker(network).check(routes)
