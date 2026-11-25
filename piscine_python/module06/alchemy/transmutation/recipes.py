"""Module 6, Part III: Transmutation recipes.

Demonstrates both absolute and relative imports within a deep package.
"""

from ..elements import create_air
from alchemy.potions import strength_potion
from elements import create_fire


def lead_to_gold() -> str:
    """Return the transmutation recipe string."""
    return (
        "Recipe transmuting Lead to Gold: brew "
        f"'{create_air()}' and '{strength_potion()}' mixed with "
        f"'{create_fire()}'"
    )
