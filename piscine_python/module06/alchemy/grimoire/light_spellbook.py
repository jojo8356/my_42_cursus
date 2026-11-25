"""Module 6, Part IV: Light spellbook.

Part of an intentional import cycle with the dark spellbook.
"""

from alchemy.grimoire.dark_spellbook import dark_spell_allowed_ingredients


def light_spell_allowed_ingredients() -> set[str]:
    """Return the set of allowed light ingredients."""
    return {"earth", "wind", "fire"}


def light_spell_record(ingredients: str) -> str:
    """Record a light spell and report the dark counterpart's ingredients."""
    dark = dark_spell_allowed_ingredients()
    return f"Light spell recorded: {ingredients} ({sorted(dark)})"
