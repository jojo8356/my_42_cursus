"""Module 6, Part IV: Avoid the Explosion - the exploding path.

Importing the light spellbook triggers the light/dark import cycle and
raises an ImportError at runtime.
"""

from alchemy.grimoire.light_spellbook import light_spell_record

print(light_spell_record("earth, wind"))
