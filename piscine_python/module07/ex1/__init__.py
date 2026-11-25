"""Module 7, Exercise 1: Capabilities.

Mixin pattern: creatures gain abilities through capability interfaces.
"""

from abc import ABC, abstractmethod


class HealCapability(ABC):
    """Interface for creatures that can heal."""

    @abstractmethod
    def heal(self, target: str = "itself") -> None:
        """Heal the target."""


class TransformCapability(ABC):
    """Interface for creatures that can transform and revert."""

    @abstractmethod
    def transform(self) -> None:
        """Transform into a stronger form."""

    @abstractmethod
    def revert(self) -> None:
        """Revert to the original form."""


class Creature:
    """A creature with a name, type and signature move."""

    def __init__(self, name: str, types: str, move: str) -> None:
        self.name = name
        self.types = types
        self.move = move

    def introduce(self) -> None:
        """Print the creature's identity."""
        print(f"{self.name} is a {self.types} type Creature")

    def attack(self) -> None:
        """Default attack behaviour."""
        print(f"{self.name} attacks normally.")


class HealingCreatureFactory:
    """Factory producing healing-capable creatures."""

    def create_base(self) -> Creature:
        return Sproutling()

    def create_evolved(self) -> Creature:
        return Bloomelle()


class TransformCreatureFactory:
    """Factory producing transform-capable creatures."""

    def create_base(self) -> Creature:
        return Shiftling()

    def create_evolved(self) -> Creature:
        return Morphagon()


class Sproutling(Creature, HealCapability):
    """A base grass creature that can heal."""

    def __init__(self) -> None:
        super().__init__("Sproutling", "Grass", "Vine Whip")

    def attack(self) -> None:
        print(f"{self.name} uses {self.move}!")

    def heal(self, target: str = "itself") -> None:
        print(f"{self.name} heals itself for a small amount")


class Bloomelle(Creature, HealCapability):
    """An evolved grass/fairy creature that heals strongly."""

    def __init__(self) -> None:
        super().__init__("Bloomelle", "Grass/Fairy", "Petal Dance")

    def attack(self) -> None:
        print(f"{self.name} uses {self.move}!")

    def heal(self, target: str = "itself") -> None:
        print(f"{self.name} heals itself and others for a large amount")


class Shiftling(Creature, TransformCapability):
    """A normal creature that can shift into a sharper form."""

    def __init__(self) -> None:
        super().__init__("Shiftling", "Normal", "normal")
        self.transformed = False

    def attack(self) -> None:
        if self.transformed:
            print(f"{self.name} performs a boosted strike!")
        else:
            print(f"{self.name} attacks normally.")

    def transform(self) -> None:
        self.transformed = True
        print(f"{self.name} shifts into a sharper form!")

    def revert(self) -> None:
        self.transformed = False
        print(f"{self.name} returns to normal.")


class Morphagon(Creature, TransformCapability):
    """A normal/dragon creature that can morph into a battle form."""

    def __init__(self) -> None:
        super().__init__("Morphagon", "Normal/Dragon", "normal")
        self.transformed = False

    def attack(self) -> None:
        if self.transformed:
            print(f"{self.name} unleashes a devastating morph strike!")
        else:
            print(f"{self.name} attacks normally.")

    def transform(self) -> None:
        self.transformed = True
        print(f"{self.name} morphs into a dragonic battle form!")

    def revert(self) -> None:
        self.transformed = False
        print(f"{self.name} stabilizes its form.")
