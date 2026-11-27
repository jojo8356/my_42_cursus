"""Planner tests: reservation routing, priority preference, restrictions."""
from __future__ import annotations

from pathlib import Path

import pytest

from src.models import Network
from src.parser import MapParser
from src.planner import ReservationRouter, ScheduleError
from src.simulation import ReplayChecker


def _network(text: str) -> Network:
    """Parse ``text`` into a network via a temporary file."""
    import tempfile

    with tempfile.NamedTemporaryFile(
        "w", suffix=".txt", delete=False, encoding="utf-8"
    ) as handle:
        handle.write(text)
        path = Path(handle.name)
    return MapParser().parse(path)


def test_plans_all_drones() -> None:
    """Every drone receives a route that ends at the end hub."""
    network = _network(
        "nb_drones: 5\nstart_hub: s 0 0\nhub: a 1 0\nend_hub: e 2 0\n"
        "connection: s-a [max_link_capacity=3]\nconnection: a-e [max_link_capacity=3]\n"
    )
    routes = ReservationRouter(network).plan()
    assert len(routes) == 5
    for route in routes:
        assert route[0].source == "s"
        assert route[-1].target == "e"


def test_routes_pass_replay_checker() -> None:
    """Planned routes always satisfy the independent validator."""
    network = _network(
        "nb_drones: 6\nstart_hub: s 0 0\nhub: a 1 0 [max_drones=2]\n"
        "hub: b 2 0 [zone=restricted max_drones=2]\nend_hub: e 3 0\n"
        "connection: s-a\nconnection: a-b\nconnection: b-e\n"
    )
    routes = ReservationRouter(network).plan()
    ReplayChecker(network).check(routes)  # must not raise


def test_priority_zone_preferred_on_equal_cost() -> None:
    """When two routes tie on turns, the priority zone is chosen."""
    network = _network(
        "nb_drones: 1\nstart_hub: s 0 0\n"
        "hub: slow 1 0 [zone=restricted]\nhub: fast 1 0 [zone=priority]\n"
        "end_hub: e 2 0\n"
        "connection: s-slow\nconnection: s-fast\n"
        "connection: slow-e\nconnection: fast-e\n"
    )
    routes = ReservationRouter(network).plan()
    visited = [step.target for step in routes[0]]
    # The priority route (s-fast-e) is preferred over the restricted one.
    assert visited == ["fast", "e"]


def test_restricted_move_takes_two_turns() -> None:
    """Entering a restricted zone spends two turns on the link."""
    network = _network(
        "nb_drones: 1\nstart_hub: s 0 0\nhub: r 1 0 [zone=restricted]\n"
        "end_hub: e 2 0\nconnection: s-r\nconnection: r-e\n"
    )
    routes = ReservationRouter(network).plan()
    restricted_steps = [s for s in routes[0] if s.target == "r"]
    assert len(restricted_steps) == 1
    step = restricted_steps[0]
    assert step.arrival - step.departure == 2


def test_capacity_forces_staggered_arrival() -> None:
    """A single-capacity bottleneck staggers drones across turns."""
    network = _network(
        "nb_drones: 3\nstart_hub: s 0 0\nhub: bottle 1 0 [max_drones=1]\n"
        "end_hub: e 2 0\nconnection: s-bottle\nconnection: bottle-e\n"
    )
    routes = ReservationRouter(network).plan()
    arrivals = [
        step.arrival for route in routes for step in route if step.target == "bottle"
    ]
    assert arrivals == sorted(arrivals)
    assert len(set(arrivals)) == 3  # no two share the same arrival turn


def test_unreachable_end_raises() -> None:
    """A drone that cannot reach the end raises a schedule error."""
    # The parser rejects an unreachable end, so build a network directly.
    from src.models import Network, Zone

    network = Network()
    network.drones = 1
    network.start = "s"
    network.end = "e"
    network.zones["s"] = Zone("s", 0, 0)
    network.zones["e"] = Zone("e", 1, 0, kind="blocked")
    network.neighbors["s"] = []
    network.neighbors["e"] = []
    with pytest.raises(ScheduleError):
        ReservationRouter(network).plan()


def test_routes_are_contiguous_and_ordered() -> None:
    """Each route is a contiguous, time-ordered sequence of steps."""
    network = _network(
        "nb_drones: 4\nstart_hub: s 0 0\nhub: a 1 0\nhub: b 2 0\n"
        "end_hub: e 3 0\nconnection: s-a\nconnection: a-b\nconnection: b-e\n"
    )
    routes = ReservationRouter(network).plan()
    for route in routes:
        for previous, current in zip(route, route[1:]):
            assert current.departure == previous.arrival
            assert current.source == previous.target
        assert route[0].departure == 0
