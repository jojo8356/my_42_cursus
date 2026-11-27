"""Turn-by-turn simulation and independent route validation.

Given the routes produced by the planner, this module derives the per-turn
movement log in the subject output format and provides a strict, independent
validator.  The validator checks the planned schedule statically and per
turn: route contiguity, zone and link capacities, two-turn restricted
transit, and complete delivery — all without trusting the planner's own
internal reservations.
"""
from __future__ import annotations

from src.models import Network, Step


class ValidationError(ValueError):
    """Raised when a planned route set violates a simulation rule."""


def build_turns(routes: list[list[Step]]) -> dict[int, list[tuple[int, str]]]:
    """Map each simulation turn to the ``(drone, token)`` movements.

    A drone moving into a restricted zone is reported twice: once in flight
    on the connection (``D<id>-<src>-<dst>``) during the intermediate turn,
    then arriving (``D<id>-<dst>``).  Waits and 1-turn moves only emit an
    arrival token.
    """
    turns: dict[int, list[tuple[int, str]]] = {}
    for drone, route in enumerate(routes, 1):
        for step in route:
            if step.link is None:
                continue
            if step.is_transit:
                turns.setdefault(step.departure + 1, []).append(
                    (drone, f"{step.source}-{step.target}")
                )
            turns.setdefault(step.arrival, []).append((drone, step.target))
    return turns


def total_turns(turns: dict[int, list[tuple[int, str]]]) -> int:
    """Return the last turn on which any drone moves."""
    return max(turns) if turns else 0


class ReplayChecker:
    """Independently validate a set of planned routes against the rules."""

    def __init__(self, network: Network) -> None:
        """Store the network to validate against."""
        self.network = network

    def check(self, routes: list[list[Step]]) -> None:
        """Validate every route and every capacity constraint."""
        if len(routes) != self.network.drones:
            raise ValidationError(
                f"expected {self.network.drones} routes, got {len(routes)}"
            )

        # 1. Per-drone structural validation.
        for number, route in enumerate(routes, 1):
            self._check_route(number, route)

        # 2. Aggregate occupancy and link usage per turn across all drones.
        zone_use: dict[tuple[str, int], int] = {}
        link_use: dict[tuple[tuple[str, str], int], int] = {}
        delivered = 0
        for route in routes:
            for step in route:
                if step.link is not None:
                    for tick in range(step.departure + 1, step.arrival + 1):
                        key = (step.link.key, tick)
                        link_use[key] = link_use.get(key, 0) + 1
                if step.target == self.network.end:
                    delivered += 1
                zone_use[(step.target, step.arrival)] = (
                    zone_use.get((step.target, step.arrival), 0) + 1
                )

        # 3. Capacity enforcement.
        for (zone_name, turn), count in zone_use.items():
            if self.network.is_exempt(zone_name):
                continue
            capacity = self.network.zone_capacity(zone_name)
            if count > capacity:
                raise ValidationError(
                    f"zone {zone_name} holds {count} drones at turn {turn} "
                    f"(capacity {capacity})"
                )

        for (link_key, tick), count in link_use.items():
            first, second = link_key
            capacity = self._link_capacity(first, second)
            if count > capacity:
                raise ValidationError(
                    f"link {link_key} carries {count} drones at turn {tick} "
                    f"(capacity {capacity})"
                )

        if delivered != self.network.drones:
            raise ValidationError(
                f"only {delivered}/{self.network.drones} drones delivered"
            )

    def _link_capacity(self, first: str, second: str) -> int:
        """Return the capacity of the link between two zones (0 if none)."""
        for _, link in self.network.neighbors.get(first, []):
            if link.key == (min(first, second), max(first, second)):
                return link.capacity
        return 0

    def _check_route(self, number: int, route: list[Step]) -> None:
        """Validate a single drone's route structure and movement rules."""
        if not route:
            raise ValidationError(f"D{number} has an empty route")
        if route[0].source != self.network.start:
            raise ValidationError(f"D{number} does not start at {self.network.start}")
        if route[-1].target != self.network.end:
            raise ValidationError(f"D{number} does not end at {self.network.end}")

        previous: Step | None = None
        for step in route:
            if previous is not None:
                if step.departure != previous.arrival:
                    raise ValidationError(
                        f"D{number} route is not contiguous "
                        f"(gap at {step.source})"
                    )
                if step.source != previous.target:
                    raise ValidationError(
                        f"D{number} route jumps from {previous.target} to {step.source}"
                    )
            zone = self.network.zones[step.target]
            if zone.kind == "blocked":
                raise ValidationError(f"D{number} routes through blocked {step.target}")

            if step.link is None:
                # Waiting in place: must stay on the current zone.
                if step.source != step.target:
                    raise ValidationError(f"D{number} waits on a wrong zone")
                if step.arrival - step.departure != 1:
                    raise ValidationError(f"D{number} wait has a bad duration")
            else:
                # A real movement: the link must connect source and target.
                endpoints = (step.link.first, step.link.second)
                if step.source not in endpoints or step.target not in endpoints:
                    raise ValidationError(f"D{number} step is not adjacent")
                duration = step.arrival - step.departure
                if duration == 2:
                    if zone.kind != "restricted":
                        raise ValidationError(
                            f"D{number} uses a 2-turn move to a non-restricted "
                            f"{step.target}"
                        )
                elif duration == 1:
                    if zone.kind == "restricted":
                        raise ValidationError(
                            f"D{number} enters restricted {step.target} in 1 turn"
                        )
                else:
                    raise ValidationError(f"D{number} step has a bad duration")
            previous = step
