#!/usr/bin/env python3
"""Reference model of the game rules, used to derive expected test values.

The gameplay tests assert exact move sequences. Hand-computing them is how test
suites get wrong expectations that are then "fixed" by weakening the assertion.
This script replays a key script against an independent implementation of the
rules written straight from chapter IV of the subject:

  * a move into a wall is refused and does not count;
  * the exit is refused while collectibles remain;
  * stepping on the last collectible and then on the exit wins;
  * ESC ends the game and later keys do nothing.

Usage:
    python3 tests/tools/reference_model.py tests/cases/maps/ref.ber "D D D S S D"
"""

import sys


def load(path):
    with open(path) as fh:
        grid = [line.rstrip("\n") for line in fh if line != ""]
    width = len(grid[0])
    for y, row in enumerate(grid):
        if len(row) != width:
            raise SystemExit(f"map is not rectangular: row {y} is {len(row)}")
    if grid[0].strip("1") or grid[-1].strip("1"):
        raise SystemExit("map is not closed by walls")
    flat = "".join(grid)
    for symbol in "CEP":
        if flat.count(symbol) < 1:
            raise SystemExit(f"map has no {symbol}")
    return grid, width


def check_reachable(grid):
    """Every collectible and the exit must be reachable from the player."""
    height = len(grid)
    width = len(grid[0])
    start = [(x, y) for y in range(height) for x in range(width)
             if grid[y][x] == "P"][0]
    seen, stack = {start}, [start]
    while stack:
        x, y = stack.pop()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < width and 0 <= ny < height):
                continue
            if (nx, ny) in seen or grid[ny][nx] == "1":
                continue
            seen.add((nx, ny))
            stack.append((nx, ny))
    return [(x, y) for y in range(height) for x in range(width)
            if grid[y][x] in "CE" and (x, y) not in seen]


DELTA = {"W": (0, -1), "S": (0, 1), "A": (-1, 0), "D": (1, 0)}


def play(grid, keys):
    """Returns (printed move counter, collected, won)."""
    height, width = len(grid), len(grid[0])
    total = "".join(grid).count("C")
    x, y = [(x, y) for y in range(height) for x in range(width)
            if grid[y][x] == "P"][0]
    collected, printed, over = 0, [], False
    for key in keys:
        if over:
            continue
        if key == "ESC":
            over = True
            continue
        if key not in DELTA:
            continue
        dx, dy = DELTA[key]
        nx, ny = x + dx, y + dy
        if not (0 <= nx < width and 0 <= ny < height):
            continue
        if grid[ny][nx] == "1":
            continue
        if grid[ny][nx] == "E" and collected < total:
            continue
        x, y = nx, ny
        if grid[ny][nx] == "C":
            collected += 1
        printed.append(len(printed) + 1)
        if grid[ny][nx] == "E" and collected == total:
            over = True
    return printed, collected, over


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    grid, _ = load(sys.argv[1])
    unreachable = check_reachable(grid)
    if unreachable:
        raise SystemExit(f"lecture impossible depuis P: {unreachable}")
    keys = sys.argv[2].split()
    printed, collected, won = play(grid, keys)
    total = "".join(grid).count("C")
    print("moves:", " ".join(str(n) for n in printed) or "(none)")
    print(f"collected: {collected}/{total}   won: {'yes' if won else 'no'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
