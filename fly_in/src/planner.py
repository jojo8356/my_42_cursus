"""Path planner for the Fly-In drone fleet.

The planner schedules one drone at a time.  For each drone it runs a
space-time Dijkstra search over ``(zone, turn)`` states: a zone may be
occupied now yet free later, so the arrival turn is part of the state.  When
a route is found it is reserved (zone occupancy and link usage per turn)
before the next drone is planned, so later drones route around the earlier
ones.

Movement cost follows the destination zone kind: 1 turn for normal/priority,
2 turns for restricted (the drone is in transit on the link during the
intermediate turn).  On equal arrival time the search prefers routes that
enter fewer non-priority zones, so priority hubs are favoured.
"""
from __future__ import annotations

from heapq import heappop, heappush

from src.models import Link, Network, Step


class ScheduleError(ValueError):
    """Raised when a drone cannot be routed to the end zone."""


class ReservationRouter:
    """Find routes with Dijkstra, then reserve each route for later drones."""

    def __init__(self, network: Network) -> None:
        """Initialize sparse zone and undirected connection reservations."""
        self.network = network
        self.occupancy: dict[tuple[str, int], int] = {}
        self.traffic: dict[tuple[tuple[str, str], int], int] = {}
        self.last_arrival: int = 0

    def plan(self) -> list[list[Step]]:
        """Plan the whole fleet, reserving each route before the next."""
        self.occupancy.clear()
        self.traffic.clear()
        self.last_arrival = 0
        routes: list[list[Step]] = []
        for _ in range(self.network.drones):
            route = self._route()
            self._reserve(route)
            routes.append(route)
        return routes

    def _space(self, name: str, time: int) -> bool:
        """Check occupancy at a turn, ignoring start/end hub limits."""
        if self.network.is_exempt(name):
            return True
        return self.occupancy.get((name, time), 0) < self.network.zone_capacity(name)

    def _link_free(self, link: Link, from_time: int, to_time: int) -> bool:
        """Check that every transit tick of a link keeps its capacity."""
        return all(
            self.traffic.get((link.key, tick), 0) < link.capacity
            for tick in range(from_time + 1, to_time + 1)
        )

    def _route(self) -> list[Step]:
        """Find the earliest arrival, preferring priority zones on ties."""
        start = self.network.start
        initial = (start, 0)
        # (arrival, penalty, zone) — heap ordered by arrival then penalty.
        queue: list[tuple[int, int, str]] = [(0, 0, start)]
        scores: dict[tuple[str, int], int] = {initial: 0}
        parents: dict[tuple[str, int], Step] = {}
        horizon = self.last_arrival + 2 * len(self.network.zones) + 1

        while queue:
            time, penalty, source = heappop(queue)
            if scores.get((source, time)) != penalty:
                continue
            if source == self.network.end:
                return self._reconstruct(parents, initial, source, time)

            # Try every neighbour, then the option of waiting in place.
            options: list[tuple[str, Link | None]] = [
                (target, link) for target, link in self.network.neighbors[source]
            ]
            options.append((source, None))

            for target, link in options:
                zone = self.network.zones[target]
                if zone.kind == "blocked":
                    continue
                duration = 2 if (link is not None and zone.kind == "restricted") else 1
                arrival = time + duration
                if arrival > horizon:
                    continue
                if not self._space(target, arrival):
                    continue
                if link is not None and not self._link_free(link, time, arrival):
                    continue

                # Penalty: entering a non-priority zone costs 1, priority
                # and waiting cost 0 — this breaks ties in favour of priority.
                cost = penalty + int(link is not None and zone.kind != "priority")
                state = (target, arrival)
                if state in scores and scores[state] <= cost:
                    continue
                scores[state] = cost
                parents[state] = Step(source, target, time, arrival, link)
                heappush(queue, (arrival, cost, target))

        raise ScheduleError("unable to schedule a route to the end zone")

    def _reconstruct(
        self,
        parents: dict[tuple[str, int], Step],
        initial: tuple[str, int],
        end_zone: str,
        end_time: int,
    ) -> list[Step]:
        """Walk parent pointers back to the start and return the route."""
        route: list[Step] = []
        state = (end_zone, end_time)
        while state != initial:
            step = parents[state]
            route.append(step)
            state = (step.source, step.departure)
        route.reverse()
        return route

    def _reserve(self, route: list[Step]) -> None:
        """Reserve zone occupancy and every tick of any restricted transit."""
        for step in route:
            self.occupancy[(step.target, step.arrival)] = (
                self.occupancy.get((step.target, step.arrival), 0) + 1
            )
            if step.link is not None:
                for tick in range(step.departure + 1, step.arrival + 1):
                    key = (step.link.key, tick)
                    self.traffic[key] = self.traffic.get(key, 0) + 1
        self.last_arrival = max(self.last_arrival, route[-1].arrival)
