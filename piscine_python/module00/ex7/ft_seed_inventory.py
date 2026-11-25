"""Module 0, Exercise 7: Seed Inventory with Type Annotations.

Report seed stock with unit-aware formatting and full type annotations.
"""


def ft_seed_inventory(name: str, count: int, unit: str) -> None:
    """Print a formatted seed inventory line according to the unit.

    Supported units are ``packets``, ``grams`` and ``area``. Any other unit
    is reported as an unknown unit type.
    """
    capitalized = name.capitalize()
    if unit == "packets":
        print(f"{capitalized} seeds: {count} packets available")
    elif unit == "grams":
        print(f"{capitalized} seeds: {count} grams total")
    elif unit == "area":
        print(f"{capitalized} seeds: covers {count} square meters")
    else:
        print("Unknown unit type")


if __name__ == "__main__":
    ft_seed_inventory("tomato", 15, "packets")
