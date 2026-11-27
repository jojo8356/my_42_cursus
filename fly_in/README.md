*This project has been created as part of the 42 curriculum by jojo8356.*

# 🚁 Fly-In — Drone Routing Simulation

## Description

**Fly-In** is a multi-drone routing simulation written in **Python 3.10+**. A
fleet of drones starts in a single **start hub** and must reach a single
**end hub** through a network of connected zones, delivering every drone in
the **fewest simulation turns** possible.

The input is a graph of **zones** (vertices) joined by **connections**
(undirected, weighted edges). Each zone and connection carries constraints:

- **Zone types** — `normal` (1 turn), `priority` (1 turn, preferred in
  pathfinding), `restricted` (2 turns, the drone is in transit on the link
  during the intermediate turn) and `blocked` (impassable).
- **Capacity** — each zone holds at most `max_drones` drones (default 1) and
  each connection carries at most `max_link_capacity` drones per turn
  (default 1). The **start and end hubs are exempt** from occupancy limits.
- **Contention** — two drones may not occupy the same zone or traverse the
  same connection on the same turn unless capacity allows it; deadlocks are
  avoided through turn-by-turn scheduling.

The project is **fully object-oriented and type-safe** (flake8 + mypy,
including `--strict`). No graph or pathfinding library is used — the graph,
Dijkstra search and reservations are implemented from scratch on the standard
library.

## Instructions

### Requirements

- Python 3.10+
- Dev tools: `pytest`, `flake8`, `mypy` (see `requirements.txt`)

### Install

```bash
make install            # install dependencies into the current environment
# or, with a virtual environment:
python3 -m venv .venv && source .venv/bin/activate && make install
```

### Run

```bash
make run                                  # interactive map menu
make run MAP=maps/easy/01_linear_path.txt
python3 main.py maps/hard/03_ultimate_challenge.txt
python3 main.py maps/easy/01_linear_path.txt --plain   # no ANSI colors
```

`--plain` disables the colored terminal output (stdout then contains only the
subject movement log). Stdout carries only the movement lines; menus and
errors go to stderr.

### Debug, lint, test

```bash
make debug          # run under the Python debugger (pdb)
make lint           # flake8 + mypy (subject-required flags)
make lint-strict    # flake8 + mypy --strict
make test           # full pytest suite
make clean          # remove bytecode and tool caches
```

## Algorithm & Implementation Strategy

The routing pipeline is: **Parse → Plan → Validate → Render**.

1. **Parser** (`src/parser.py`) — reads the map, validates every definition
   against the subject rules (unique names, valid zone types, positive
   capacities, no duplicate connections, reachable end) and builds a
   `Network`. Errors carry the offending line number and cause.
2. **Planner** (`src/planner.py`) — a **space-time Dijkstra with per-turn
   reservations**. Each drone is routed one at a time over `(zone, turn)`
   states: a zone may be occupied now yet free later, so the arrival turn is
   part of the state. The search finds the earliest arrival given the routes
   already planned, then reserves that route (zone occupancy and every link
   transit tick) before planning the next drone. On equal arrival time it
   prefers routes entering fewer non-priority zones, so priority hubs win
   ties. This is a greedy fleet strategy: it meets all mandatory targets and
   solves the Challenger map.
3. **Validator** (`src/simulation.py`) — an **independent replay checker**
   that re-derives the schedule and verifies every rule without trusting the
   planner's reservations: route contiguity, adjacency, zone and link
   capacities, exact two-turn restricted transit and complete delivery.
4. **Renderer** (`src/renderer.py`) — prints one movement line per turn in
   the subject format, colored by the destination zone.

**Complexity.** For a fleet of `N` drones, `V` zones, `E` connections and a
search horizon `H`, planning is `O(N · H · (V + E) · log(V·H))` time and
`O(V·H)` temporary state, plus stored routes and sparse reservations. Routes
are computed once, then played back.

## Output Format

Each simulation turn is one line; drones that do not move are omitted. A
move into a restricted zone is shown in flight (`D<id>-<src>-<dst>`) on the
intermediate turn, then arriving (`D<id>-<dst>`):

```text
D1-waypoint1
D1-waypoint2 D2-waypoint1
D1-goal D2-waypoint2
D2-goal
```

## Visual Representation

The renderer provides **colored terminal output**: every movement token is
painted with the color declared on its destination zone (a small ANSI
palette). Colors are cosmetic and can be disabled with `--plain`; the
underlying text is always the exact subject format, so output can be
redirected to a file cleanly.

## Performance Benchmarks

All supplied maps are solved and validated by the test suite. Reference
targets vs. this implementation:

| Map | Target | Achieved |
| :--- | ---: | ---: |
| Easy — linear path | ≤ 6 | **4** |
| Easy — simple fork | ≤ 8 | **4** |
| Easy — basic capacity | ≤ 6 | **4** |
| Medium — dead end trap | ≤ 12 | **8** |
| Medium — circular loop | ≤ 15 | **15** |
| Medium — priority puzzle | ≤ 12 | **7** |
| Hard — maze nightmare | ≤ 30 | **13** |
| Hard — capacity hell | ≤ 35 | **16** |
| Hard — ultimate challenge | ≤ 45 | **26** |
| **Challenger — the impossible dream** | **record 45** | **43** ✅ |

The optional **Challenger** map is solved in **43 turns**, beating the
45-turn reference record (bonus).

## Testing

The suite (`make test`, 205 cases) is written to enterprise standards and
covers:

- **Parser** — every acceptance/rejection rule, edge cases and line-numbered
  errors, plus fuzzing with random malformed inputs (no unexpected crashes).
- **Planner** — reservation routing, priority preference, restricted
  two-turn moves, capacity staggering and contiguity.
- **Simulation** — turn mechanics, output format, and the replay checker
  rejecting over-capacity zones, non-adjacent routes, bad restricted moves
  and incomplete delivery.
- **Maps** — all 10 supplied maps solved and validated, benchmark targets
  and the Challenger bonus.
- **Stress** — 40 seeded random networks solved and validated, a 200-drone
  fleet, and 100 fuzz iterations.

## Resources

- Python `heapq` (priority queue) and the standard library.
- Dijkstra, E. W. (1959), *A note on two problems in connexion with graphs*.
- Silver, D. (2005), *Cooperative Pathfinding*, AIIDE.
- The supplied **Fly-In subject v1.5/v1.6** and its map archive.

### Use of AI

AI tools were used to help interpret the subject, design the parser and
space-time planner, write the independent replay checker and test suite, and
review the code. Every generated piece was understood, reviewed and tested
before being committed; the routing logic, parser rules and validation were
all verified against the subject and the supplied maps.
