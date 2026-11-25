"""Module 6, Part II: Distillation.

Brew potions by combining element creation results.
"""

from alchemy.elements import create_air, create_earth
from elements import create_fire, create_water


def healing_potion() -> str:
    """Brew a healing potion from earth and air elements."""
    return f"Healing potion brewed with '{create_earth()}' and '{create_air()}'"


def strength_potion() -> str:
    """Brew a strength potion from fire and water elements."""
    return f"Strength potion brewed with '{create_fire()}' and '{create_water()}'"
