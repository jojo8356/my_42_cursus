"""Module 3, Exercise 2: Position Tracker.

Read 3D coordinates from standard input and compute distances.
"""

import math
import sys


def get_player_pos() -> tuple[float, float, float]:
    """Read a 3D coordinate line and return it as a tuple of floats."""
    line = sys.stdin.readline().strip()
    parts = [float(coordinate) for coordinate in line.split(",")]
    return (parts[0], parts[1], parts[2])


def main() -> None:
    """Read two coordinates and report their distances."""
    first = get_player_pos()
    print(f"Got a first tuple: {first}")

    second_line = sys.stdin.readline().strip()
    second_parts = [float(coordinate) for coordinate in second_line.split(",")]
    second = (second_parts[0], second_parts[1], second_parts[2])

    distance_to_center = math.sqrt(sum(coordinate * coordinate for coordinate in first))
    print(f"Distance to center: {distance_to_center:.4f}")

    distance = math.sqrt(
        sum((a - b) ** 2 for a, b in zip(first, second))
    )
    print(f"Distance between the 2 sets of coordinates: {distance:.4f}")


if __name__ == "__main__":
    main()
