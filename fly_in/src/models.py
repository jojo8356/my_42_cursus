"""Data models for the Fly-In drone routing simulation.

This module defines the typed, object-oriented building blocks used by the
parser, the path planner and the simulator: zones, bidirectional links, the
parsed network graph, a single movement step and a drone in flight.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

# Zone kinds allowed by the subject.
ZONE_KINDS = ("normal", "priority", "restricted", "blocked")

# Cost in turns to enter a zone of a given kind.  Blocked zones are never
# entered, so they carry an infinite cost here.
ZONE_COST = {
    "normal": 1,
    "priority": 1,
    "restricted": 2,
    "blocked": float("inf"),
}


class Zone:
    """A named graph vertex with a position, a kind, a color and a capacity."""

    def __init__(
        self,
        name: str,
        x: int,
        y: int,
        kind: str = "normal",
        color: str = "none",
        capacity: int = 1,
    ) -> None:
        """Store the zone's coordinates, type, color and occupancy limit."""
        self.name = name
        self.x = x
        self.y = y
        self.kind = kind
        self.color = color
        self.capacity = capacity

    @property
    def cost(self) -> float:
        """Return the movement cost (in turns) to enter this zone."""
        return ZONE_COST[self.kind]

    def __repr__(self) -> str:
        """Return a compact, debug-friendly representation."""
        return f"Zone({self.name!r}, {self.kind}, cap={self.capacity})"


class Link:
    """An undirected connection shared by both travel directions."""

    def __init__(
        self,
        first: str,
        second: str,
        capacity: int = 1,
    ) -> None:
        """Store the endpoints and the shared connection capacity."""
        self.first = first
        self.second = second
        self.capacity = capacity

    @property
    def key(self) -> tuple[str, str]:
        """Return the canonical reservation identifier (order-independent)."""
        return (min(self.first, self.second), max(self.first, self.second))

    def __repr__(self) -> str:
        """Return a compact, debug-friendly representation."""
        return f"Link({self.first!r}-{self.second!r}, cap={self.capacity})"


class Network:
    """The parsed graph and fleet configuration."""

    def __init__(self) -> None:
        """Initialize an empty network with its own dictionaries."""
        self.drones: int = 0
        self.start: str = ""
        self.end: str = ""
        self.zones: dict[str, Zone] = {}
        # Adjacency list: zone name -> list of (neighbour name, link).
        self.neighbors: dict[str, list[tuple[str, Link]]] = {}

    def zone_capacity(self, name: str) -> int:
        """Return the effective occupancy limit of a zone.

        The start and end hubs are exempt from capacity limits: every drone
        may share the start zone and any number may be delivered to the end.
        """
        if name in (self.start, self.end):
            return self.drones
        return self.zones[name].capacity

    def is_exempt(self, name: str) -> bool:
        """Return whether a zone ignores occupancy limits (start/end hubs)."""
        return name in (self.start, self.end)


@dataclass
class Step:
    """One movement (or wait) in a planned route.

    A step records where a drone leaves from and arrives, the departure and
    arrival turns, and the link traversed (``None`` for a wait).  For a move
    into a restricted zone the arrival is two turns after the departure and
    the drone is considered in transit on the link during the intermediate
    turn.
    """

    source: str
    target: str
    departure: int
    arrival: int
    link: Optional[Link] = None

    @property
    def is_transit(self) -> bool:
        """Return whether this step spends two turns (restricted flight)."""
        return self.link is not None and (self.arrival - self.departure) == 2


class Drone:
    """A single drone identified by its number and carrying a planned route."""

    def __init__(self, number: int, route: list[Step]) -> None:
        """Store the drone identifier and its planned list of steps."""
        self.number = number
        self.route = route
