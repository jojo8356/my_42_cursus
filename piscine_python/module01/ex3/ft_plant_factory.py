"""Module 1, Exercise 3: Plant Factory.

Create many plants at once through the constructor and report each creation.
"""


class Plant:
    """A plant created through a parameterised constructor."""

    def __init__(self, name: str, height: int, age: int) -> None:
        self.name = name
        self.height = height
        self.age = age

    def show(self) -> None:
        """Print a one-line summary of the plant."""
        print(f"{self.name}: {self.height}cm, {self.age} days old")


def main() -> None:
    seeds = [
        ("Rose", 30, 5),
        ("Tulip", 20, 3),
        ("Oak", 50, 10),
        ("Fern", 15, 2),
        ("Maple", 40, 8),
    ]
    plants = []
    for name, height, age in seeds:
        plant = Plant(name, height, age)
        plants.append(plant)
        print(f"Created: {plant.name}")
    for plant in plants:
        plant.show()


if __name__ == "__main__":
    main()
