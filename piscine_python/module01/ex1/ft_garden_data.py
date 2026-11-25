"""Module 1, Exercise 1: Garden Data Organizer.

A first class modelling a plant with a show method, and a garden holding
several plants.
"""


class Plant:
    """A simple plant with a name, height and age."""

    def __init__(self, name: str, height: int, age: int) -> None:
        self.name = name
        self.height = height
        self.age = age

    def show(self) -> None:
        """Print a one-line summary of the plant."""
        print(f"{self.name}: {self.height}cm, {self.age} days old")


def main() -> None:
    rose = Plant("Rose", 30, 5)
    tulip = Plant("Tulip", 20, 3)
    oak = Plant("Oak", 50, 10)
    for plant in (rose, tulip, oak):
        plant.show()


if __name__ == "__main__":
    main()
