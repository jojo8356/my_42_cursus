"""Module 7, Exercise 2: Abstract Strategy.

Strategy pattern with an abstract battle strategy and concrete variants.
"""

from abc import ABC, abstractmethod


class BattleStrategy(ABC):
    """Abstract strategy defining battle behaviour."""

    @abstractmethod
    def act(self, creature: object) -> str:
        """Return the action performed by the strategy."""

    @abstractmethod
    def is_valid(self, creature: object) -> bool:
        """Return whether the strategy suits the creature."""


class NormalStrategy(BattleStrategy):
    """A balanced strategy valid for any creature."""

    def is_valid(self, creature: object) -> bool:
        return True

    def act(self, creature: object) -> str:
        return f"{creature} attacks normally"


class AggressiveStrategy(BattleStrategy):
    """A strategy only valid for aggressive creatures."""

    def is_valid(self, creature: object) -> bool:
        return bool(getattr(creature, "aggressive", False))

    def act(self, creature: object) -> str:
        return f"{creature} strikes aggressively"


class DefensiveStrategy(BattleStrategy):
    """A cautious strategy valid for any creature."""

    def is_valid(self, creature: object) -> bool:
        return True

    def act(self, creature: object) -> str:
        return f"{creature} defends"
