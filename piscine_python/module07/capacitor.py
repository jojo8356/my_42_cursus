"""Module 7, Exercise 1: Capacitor scene demonstrating the mixins."""

from ex1 import Bloomelle, Morphagon, Shiftling, Sproutling


def main() -> None:
    """Demonstrate healing and transform capabilities."""
    sproutling = Sproutling()
    sproutling.introduce()
    sproutling.attack()
    sproutling.heal()

    bloomelle = Bloomelle()
    bloomelle.introduce()
    bloomelle.attack()
    bloomelle.heal()

    shiftling = Shiftling()
    shiftling.introduce()
    shiftling.attack()
    shiftling.transform()
    shiftling.attack()
    shiftling.revert()

    morphagon = Morphagon()
    morphagon.introduce()
    morphagon.attack()
    morphagon.transform()
    morphagon.attack()
    morphagon.revert()


if __name__ == "__main__":
    main()
