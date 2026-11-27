"""Map-level tests: every supplied map, benchmark targets and the bonus.

Each map is parsed, planned, validated by the independent replay checker and
its turn count is compared against the subject's performance targets.  The
optional Challenger map must beat the 45-turn reference record.
"""
from __future__ import annotations

from pathlib import Path

import pytest

from src.models import Network, Step
from src.parser import MapParser
from src.planner import ReservationRouter
from src.renderer import MovementRenderer
from src.simulation import ReplayChecker, build_turns, total_turns

# (category/file, subject target in turns, is_challenger).
BENCHMARKS: list[tuple[str, int, bool]] = [
    ("easy/01_linear_path.txt", 6, False),
    ("easy/02_simple_fork.txt", 8, False),
    ("easy/03_basic_capacity.txt", 6, False),
    ("medium/01_dead_end_trap.txt", 12, False),
    ("medium/02_circular_loop.txt", 15, False),
    ("medium/03_priority_puzzle.txt", 12, False),
    ("hard/01_maze_nightmare.txt", 30, False),
    ("hard/02_capacity_hell.txt", 35, False),
    ("hard/03_ultimate_challenge.txt", 45, False),
    ("challenger/01_the_impossible_dream.txt", 45, True),
]


def _solve(map_root: Path, relative: str) -> tuple[Network, list[list[Step]], int]:
    """Parse, plan and validate a map; return (network, routes, turns)."""
    network = MapParser().parse(map_root / relative)
    routes = ReservationRouter(network).plan()
    ReplayChecker(network).check(routes)
    turns = total_turns(build_turns(routes))
    return network, routes, turns


@pytest.mark.parametrize("relative,target,is_challenger", BENCHMARKS)
def test_map_meets_target(
    map_root: Path, relative: str, target: int, is_challenger: bool
) -> None:
    """Every map is solved in at most its subject turn target."""
    network, _routes, turns = _solve(map_root, relative)
    assert network.drones > 0
    if is_challenger:
        # Bonus: beat the 45-turn reference record (strictly fewer).
        assert turns < target, f"challenger solved in {turns}, record is {target}"
    else:
        assert turns <= target, f"{relative} solved in {turns}, target {target}"


@pytest.mark.parametrize("relative,target,is_challenger", BENCHMARKS)
def test_map_output_format(
    map_root: Path, relative: str, target: int, is_challenger: bool
) -> None:
    """The rendered log is well-formed: one line per turn, valid tokens."""
    network, routes, turns = _solve(map_root, relative)
    text = MovementRenderer(network, color=False).render(routes)
    lines = text.splitlines()
    assert len(lines) == turns
    for line in lines:
        if not line.strip():
            continue
        for token in line.split():
            assert token.startswith("D")
            assert "-" in token


def test_challenger_beats_reference_record(map_root: Path) -> None:
    """The optional Challenger map is solved strictly under 45 turns."""
    _network, _routes, turns = _solve(
        map_root, "challenger/01_the_impossible_dream.txt"
    )
    assert turns < 45


def test_all_mandatory_maps_deliver_every_drone(map_root: Path) -> None:
    """On every mandatory map the replay checker confirms full delivery."""
    for relative, _target, is_challenger in BENCHMARKS:
        if is_challenger:
            continue
        network = MapParser().parse(map_root / relative)
        routes = ReservationRouter(network).plan()
        # The checker raises if any drone is not delivered.
        ReplayChecker(network).check(routes)


def test_deterministic_repeated_runs(map_root: Path) -> None:
    """Planning is deterministic: repeated runs yield the same turn count."""
    first = _solve(map_root, "hard/03_ultimate_challenge.txt")[2]
    second = _solve(map_root, "hard/03_ultimate_challenge.txt")[2]
    assert first == second
