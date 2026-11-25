"""Module 6, Part IV: Avoid the Explosion - the safe path.

Imports only the standalone validator, so it runs without hitting the cycle.
"""

from alchemy.grimoire.light_validator import validate_ingredients

result = validate_ingredients("Earth, wind and fire")
print(f"Spell recorded: {result}")
