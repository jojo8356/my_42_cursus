"""Module 7, Exercise 0: Creature Factory.

Abstract Factory pattern with concrete fire and water factories.
"""

from abc import ABC, abstractmethod


class Creature(ABC):
    """Abstract creature with a type and a signature move."""

    def __init__(self, name: str, types: str, move: str) -> None:
        self.name = name
        self.types = types
        self.move = move

    @abstractmethod
    def attack(self) -> None:
        """Perform the creature's signature move."""

    def introduce(self) -> None:
        """Print the creature's identity."""
        print(f"{self.name} is a {self.types} type Creature")


class CreatureFactory(ABC):
    """Abstract factory for creating creatures."""

    @abstractmethod
    def create_base(self) -> Creature:
        """Create a base-stage creature."""

    @abstractmethod
    def create_evolved(self) -> Creature:
        """Create an evolved creature."""


class FlameFactory(CreatureFactory):
    """Factory producing fire-type creatures."""

    def create_base(self) -> Creature:
        return Flameling()

    def create_evolved(self) -> Creature:
        return Pyrodon()


class AquaFactory(CreatureFactory):
    """Factory producing water-type creatures."""

    def create_base(self) -> Creature:
        return Aquabub()

    def create_evolved(self) -> Creature:
        return Torragon()


class Flameling(Creature):
    """A base fire creature."""

    def __init__(self) -> None:
        super().__init__("Flameling", "Fire", "Ember")

    def attack(self) -> None:
        print(f"{self.name} uses {self.move}!")


class Pyrodon(Creature):
    """An evolved fire/flying creature."""

    def __init__(self) -> None:
        super().__init__("Pyrodon", "Fire/Flying", "Flamethrower")

    def attack(self) -> None:
        print(f"{self.name} uses {self.move}!")


class Aquabub(Creature):
    """A base water creature."""

    def __init__(self) -> None:
        super().__init__("Aquabub", "Water", "Water Gun")

    def attack(self) -> None:
        print(f"{self.name} uses {self.move}!")


class Torragon(Creature):
    """An evolved water creature."""

    def __init__(self) -> None:
        super().__init__("Torragon", "Water", "Hydro Pump")

    def attack(self) -> None:
        print(f"{self.name} uses {self.move}!")
