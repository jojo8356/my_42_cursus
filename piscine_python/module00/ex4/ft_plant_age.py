"""Module 0, Exercise 4: Plant Age Check.

Decide whether a plant is mature enough to be harvested based on its age.
"""


def ft_plant_age() -> None:
    """Read the plant age and report whether it is ready to harvest."""
    age = int(input().strip())
    if age >= 60:
        print("Plant is ready to harvest!")
    else:
        print("Plant needs more time to grow.")


if __name__ == "__main__":
    ft_plant_age()
