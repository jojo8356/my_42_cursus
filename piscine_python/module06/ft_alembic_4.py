"""Module 6, Part I: attempt to access a non-exported symbol.

create_earth is intentionally not re-exported by the alchemy package, so
accessing it as an attribute raises an AttributeError.
"""

import alchemy

print(alchemy.create_earth())
