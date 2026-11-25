"""Module 7, Exercise 2: Tournament scene using the strategy pattern."""

from ex2 import AggressiveStrategy, DefensiveStrategy, NormalStrategy


class Creature:
    """A battle-ready creature, optionally aggressive."""

    def __init__(self, name: str, aggressive: bool = False) -> None:
        self.name = name
        self.aggressive = aggressive

    def __str__(self) -> str:
        return self.name


def run_tournament(label: str, strategy, creatures: list) -> None:
    """Run a single tournament with the given strategy and creatures."""
    print(f"Tournament {label}")
    print("*** Tournament ***")
    print(f"{len(creatures)} opponents involved")
    for creature in creatures:
        if not strategy.is_valid(creature):
            print(f"Invalid Creature {creature} for this aggressive strategy")
            continue
        print("* Battle *")
        print(strategy.act(creature))
        print("now fight!")


def main() -> None:
    """Run three tournaments with different strategies."""
    tournaments = [
        ("0 (basic)", NormalStrategy(), [Creature("A")]),
        ("1 (error)", AggressiveStrategy(), [Creature("B")]),
        ("2 (multiple)", DefensiveStrategy(), [Creature("C"), Creature("D")]),
    ]
    for label, strategy, creatures in tournaments:
        run_tournament(label, strategy, creatures)


if __name__ == "__main__":
    main()
