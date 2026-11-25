"""Module 1, Exercise 2: Plant Growth Simulator.

Simulate a week of growth for a plant and report the weekly growth.
"""


class Plant:
    """A plant that can grow and age over time."""

    def __init__(self, name: str) -> None:
        self.name = name
        self.height = 10.0
        self.days = 0

    def grow(self) -> None:
        """Increase the plant's height."""
        self.height += 2.5

    def age(self) -> None:
        """Increase the plant's age by one day."""
        self.days += 1

    def show(self) -> None:
        """Print a one-line summary of the plant."""
        print(f"{self.name}: {self.height:.1f}cm, {self.days} days old")


def main() -> None:
    plant = Plant("Rose")
    start_height = plant.height
    print("== Garden Plant Growth ==")
    for day in range(1, 8):
        print(f"Day {day}")
        plant.grow()
        plant.age()
    print(f"Growth this week: {plant.height - start_height:.1f}cm")
    plant.show()


if __name__ == "__main__":
    main()
