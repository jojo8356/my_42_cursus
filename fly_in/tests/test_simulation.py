"""Simulation tests: turn mechanics, output format, replay validation."""
from __future__ import annotations

import tempfile
from pathlib import Path

import pytest

from src.models import Link, Network, Step, Zone
from src.parser import MapParser
from src.planner import ReservationRouter
from src.simulation import (
    ReplayChecker,
    ValidationError,
    build_turns,
    total_turns,
)


def _network(text: str) -> Network:
    """Parse ``text`` from a temporary file into a network."""
    with tempfile.NamedTemporaryFile(
        "w", suffix=".txt", delete=False, encoding="utf-8"
    ) as handle:
        handle.write(text)
        path = Path(handle.name)
    return MapParser().parse(path)


def _make_network() -> Network:
    """Build a tiny hand-made network for structural checks."""
    network = Network()
    network.drones = 2
    network.start = "s"
    network.end = "e"
    network.zones["s"] = Zone("s", 0, 0)
    network.zones["a"] = Zone("a", 1, 0)
    network.zones["e"] = Zone("e", 2, 0)
    link = Link("s", "a", capacity=1)
    network.neighbors["s"] = [("a", link)]
    network.neighbors["a"] = [("s", link)]
    network.neighbors["e"] = []
    return network


def test_build_turns_emits_arrival_tokens() -> None:
    """A one-turn move emits a single arrival token at its arrival turn."""
    route = [Step("s", "a", 0, 1, Link("s", "a"))]
    turns = build_turns([route])
    assert turns == {1: [(1, "a")]}
    assert total_turns(turns) == 1


def test_build_turns_emits_transit_then_arrival() -> None:
    """A restricted move emits a transit token then an arrival token."""
    network = _make_network()
    network.zones["a"] = Zone("a", 1, 0, kind="restricted")
    link = Link("s", "a")
    route = [Step("s", "a", 0, 2, link)]
    turns = build_turns([route])
    assert turns[1] == [(1, "s-a")]
    assert turns[2] == [(1, "a")]
    assert total_turns(turns) == 2


def test_build_turns_omits_waits() -> None:
    """A waiting step (no link) produces no output token."""
    route = [Step("s", "s", 0, 1, None), Step("s", "a", 1, 2, Link("s", "a"))]
    turns = build_turns([route])
    assert 1 not in turns
    assert turns[2] == [(1, "a")]


def test_checker_accepts_valid_routes() -> None:
    """A valid planned schedule passes the replay checker."""
    network = _network(
        "nb_drones: 3\nstart_hub: s 0 0\nhub: a 1 0 [max_drones=2]\n"
        "end_hub: e 2 0\nconnection: s-a\nconnection: a-e\n"
    )
    routes = ReservationRouter(network).plan()
    ReplayChecker(network).check(routes)


def test_checker_rejects_over_capacity_zone() -> None:
    """Two drones arriving at a capacity-1 zone on the same turn fail."""
    network = _network(
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nend_hub: e 2 0\n"
        "connection: s-a\nconnection: a-e\n"
    )
    # Two structurally-valid drones both arrive on "a" at turn 1 (cap 1).
    link_sa = Link("s", "a")
    routes = [
        [Step("s", "a", 0, 1, link_sa), Step("a", "e", 1, 2, Link("a", "e"))],
        [Step("s", "a", 0, 1, link_sa), Step("a", "e", 1, 2, Link("a", "e"))],
    ]
    network.drones = 2
    with pytest.raises(ValidationError, match="capacity"):
        ReplayChecker(network).check(routes)


def test_checker_rejects_non_adjacent_route() -> None:
    """A route that uses a link not matching its endpoints is rejected."""
    network = _network(
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nhub: b 2 0\n"
        "end_hub: e 3 0\nconnection: s-a\nconnection: a-b\nconnection: b-e\n"
    )
    # Route ends at e, but the first step uses link s-a to reach "b".
    route = [Step("s", "b", 0, 1, Link("s", "a")), Step("b", "e", 1, 2, Link("b", "e"))]
    with pytest.raises(ValidationError, match="not adjacent"):
        ReplayChecker(network).check([route])


def test_checker_rejects_restricted_in_one_turn() -> None:
    """Entering a restricted zone in one turn is rejected."""
    network = _network(
        "nb_drones: 1\nstart_hub: s 0 0\nhub: r 1 0 [zone=restricted]\n"
        "end_hub: e 2 0\nconnection: s-r\nconnection: r-e\n"
    )
    route = [Step("s", "r", 0, 1, Link("s", "r")), Step("r", "e", 1, 2, Link("r", "e"))]
    with pytest.raises(ValidationError, match="restricted"):
        ReplayChecker(network).check([route])


def test_checker_rejects_route_not_ending_at_end() -> None:
    """A route that does not finish at the end hub is rejected."""
    network = _network(
        "nb_drones: 1\nstart_hub: s 0 0\nhub: a 1 0\nend_hub: e 2 0\n"
        "connection: s-a\nconnection: a-e\n"
    )
    route = [Step("s", "a", 0, 1, Link("s", "a"))]
    with pytest.raises(ValidationError, match="does not end"):
        ReplayChecker(network).check([route])


def test_end_zone_has_no_capacity_limit() -> None:
    """Any number of drones may arrive at the end hub at once."""
    network = _network(
        "nb_drones: 4\nstart_hub: s 0 0\nend_hub: e 1 0\nconnection: s-e\n"
    )
    routes = ReservationRouter(network).plan()
    ReplayChecker(network).check(routes)
