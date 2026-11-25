"""Module 6, Part I: The Alchemy package.

Re-exports a curated set of symbols and an alias for potion brewing.
"""

from alchemy.elements import create_air
from alchemy.potions import healing_potion as heal

__all__ = ["create_air", "heal"]
