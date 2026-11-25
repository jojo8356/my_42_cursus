"""Module 3, Exercise 3: Achievement Hunter.

Track achievement sets per player and summarise common, unique and missing
achievements using set operations.
"""


def gen_player_achievements(name: str, achievements: set[str]) -> set[str]:
    """Return a defensive copy of a player's achievement set."""
    return set(achievements)


def main() -> None:
    """Print a per-player and cross-player achievement report."""
    raw_data = {
        "Alice": {"First Blood", "Golden Achievement"},
        "Bob": {"Survivor", "Stellar Explorer"},
        "Carol": {"Explorer", "Master Builder"},
        "Dave": {"Builder", "Legendary Hero"},
    }

    players = {
        name: gen_player_achievements(name, achievements)
        for name, achievements in raw_data.items()
    }

    for name, achievements in players.items():
        print(f"Player {name}: {achievements}")

    all_achievements = set().union(*players.values())
    print(f"All distinct achievements: {all_achievements}")

    common = set.intersection(*players.values())
    print(f"Common achievements: {common}")

    for name, achievements in players.items():
        others = set().union(*[set for n, set in players.items() if n != name])
        only = achievements - others
        if only:
            print(f"Only {name} has: {only}")
        missing = all_achievements - achievements
        if missing:
            print(f"{name} is missing: {missing}")


if __name__ == "__main__":
    main()
