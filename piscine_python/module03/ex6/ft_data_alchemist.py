"""Module 3, Exercise 6: Data Alchemist.

Transform player data with list and dictionary comprehensions.
"""


def main() -> None:
    """Apply comprehension-based transformations to a player list."""
    players = [
        {"name": "alice", "score": 92},
        {"name": "bob", "score": 74},
        {"name": "carol", "score": 88},
        {"name": "dave", "score": 65},
        {"name": "erin", "score": 95},
    ]

    print(f"Initial list of players: {players}")

    capitalized = [player["name"].capitalize() for player in players]
    print(f"New list with all names capitalized: {capitalized}")

    long_names = [name for name in capitalized if len(name) > 3]
    print(f"New list of capitalized names only: {long_names}")

    score_dict = {
        player["name"].capitalize(): player["score"] for player in players
    }
    print(f"Score dict: {score_dict}")

    average = sum(score_dict.values()) / len(score_dict)
    print(f"Score average is {average:.1f}")

    high_scores = {
        name: score for name, score in score_dict.items() if score >= 80
    }
    print(f"High scores: {high_scores}")


if __name__ == "__main__":
    main()
