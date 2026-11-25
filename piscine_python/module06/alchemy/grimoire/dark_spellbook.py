"""Module 6, Part IV: Dark spellbook.

Part of an intentional import cycle with the light spellbook.
"""

from alchemy.grimoire.light_spellbook import light_spell_allowed_ingredients


def dark_spell_allowed_ingredients() -> set[str]:
    """Return the set of allowed dark ingredients."""
    return {"shadow", "night"}


def dark_spell_record(ingredients: str) -> str:
    """Record a dark spell and report the light counterpart's ingredients."""
    light = light_spell_allowed_ingredients()
    return f"Dark spell recorded: {ingredients} ({sorted(light)})"
