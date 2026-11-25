"""Module 1, Exercise 6: Garden Analytics.

Nested statistics class, static and class methods, and a deeper inheritance
hierarchy (Seed inherits from Flower).
"""


class Plant:
    """A plant with analytics helpers and a nested statistics class."""

    class Statistique:
        """Track simple garden-wide counters."""

        def __init__(self) -> None:
            self.grown = 0
            self.aged = 0

        def summary(self) -> str:
            """Return a human-readable statistics summary."""
            return f"Stats: {self.grown} plants grown, {self.aged} aged"

    def __init__(self, name: str, height: int, age: int) -> None:
        self.name = name
        self.height = height
        self.days = age

    @staticmethod
    def is_ancient(age: int) -> bool:
        """Return True when a plant is older than 365 days."""
        return age > 365

    @classmethod
    def anonymous(cls) -> "Plant":
        """Create an anonymous plant with a placeholder name."""
        return cls("Unknown plant", 0, 0)

    def grow(self) -> None:
        """Increase the plant's height."""
        self.height += 5

    def age(self) -> None:
        """Increase the plant's age by one day."""
        self.days += 1

    def show(self) -> None:
        """Print a one-line summary of the plant."""
        print(f"{self.name}: {self.height}cm, {self.days} days old")


class Flower(Plant):
    """A flowering plant."""


class Tree(Plant):
    """A tree."""


class Vegetable(Plant):
    """A vegetable."""


class Seed(Flower):
    """A seed, which is a flowering plant in its earliest stage."""


def main() -> None:
    stats = Plant.Statistique()
    rose = Plant("Rose", 30, 400)
    print(f"is_ancient: {Plant.is_ancient(rose.days)}")
    unknown = Plant.anonymous()
    unknown.show()
    print("grow")
    rose.grow()
    print("age")
    rose.age()
    print("show")
    rose.show()
    stats.grown += 1
    stats.aged += 1
    print(stats.summary())


if __name__ == "__main__":
    main()
