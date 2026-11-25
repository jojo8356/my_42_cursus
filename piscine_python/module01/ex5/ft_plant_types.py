"""Module 1, Exercise 5: Specialized Plant Types.

Inheritance hierarchy: Plant -> Flower, Tree, Vegetable. Vegetable tracks a
nutritional value that grows over time.
"""


class Plant:
    """Base class for all plants."""

    def __init__(self, name: str) -> None:
        self.name = name
        self.height = 10
        self.days = 0

    def show(self) -> None:
        """Print a one-line summary of the plant."""
        print(f"{self.name}: {self.height}cm, {self.days} days old")


class Flower(Plant):
    """A flowering plant that can bloom."""

    def bloom(self) -> None:
        """Report that the flower is blooming."""
        print(f"{self.name} is blooming")


class Tree(Plant):
    """A tree that can produce shade."""

    def produce_shade(self) -> None:
        """Report that the tree provides shade."""
        print(f"{self.name} provides shade")


class Vegetable(Plant):
    """A vegetable that gains nutritional value as it grows."""

    def __init__(self, name: str) -> None:
        super().__init__(name)
        self.nutritional_value = 0

    def grow(self) -> None:
        """Increase height and nutritional value."""
        self.height += 5
        self.nutritional_value += 10

    def age(self) -> None:
        """Increase age and nutritional value."""
        self.days += 1
        self.nutritional_value += 5

    def show(self) -> None:
        """Print a summary followed by the nutritional value on its own line."""
        print(f"{self.name}: {self.height}cm, {self.days} days old")
        print(f"Nutritional value: {self.nutritional_value}")


def main() -> None:
    flower = Flower("Rose")
    flower.bloom()
    tree = Tree("Oak")
    tree.produce_shade()
    vegetable = Vegetable("Carrot")
    vegetable.show()
    vegetable.grow()
    vegetable.age()
    vegetable.show()


if __name__ == "__main__":
    main()
