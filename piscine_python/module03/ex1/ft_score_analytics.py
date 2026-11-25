"""Module 3, Exercise 1: Score Cruncher.

Aggregate and analyse numeric scores passed as command-line arguments.
"""

import sys


def main() -> None:
    """Validate incoming scores and print summary statistics."""
    scores = []
    for arg in sys.argv[1:]:
        try:
            scores.append(int(arg))
        except ValueError:
            print(f"Invalid parameter: '{arg}'")

    if not scores:
        print("No scores provided. Usage: ft_score_analytics.py <score> [score ...]")
        return

    print(f"Scores processed: {scores}")
    print(f"Total players: {len(scores)}")
    print(f"Total score: {sum(scores)}")
    print(f"Average score: {sum(scores) / len(scores)}")
    print(f"High score: {max(scores)}")
    print(f"Low score: {min(scores)}")
    print(f"Score range: {max(scores) - min(scores)}")


if __name__ == "__main__":
    main()
