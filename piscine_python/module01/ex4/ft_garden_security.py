"""Module 1, Exercise 4: Garden Security System.

Encapsulation with protected attributes and validated getters/setters.
"""


class Plant:
    """A plant with protected attributes and validated setters."""

    def __init__(self, name: str, height: int, age: int) -> None:
        self._name = name
        self._height = height
        self._age = age

    def set_height(self, height: int) -> None:
        """Set the height, rejecting negative values."""
        if height < 0:
            print("Height update rejected: negative value")
            return
        self._height = height

    def set_age(self, age: int) -> None:
        """Set the age, rejecting negative values."""
        if age < 0:
            print("Age update rejected: negative value")
            return
        self._age = age

    def get_height(self) -> int:
        """Return the protected height."""
        return self._height

    def get_age(self) -> int:
        """Return the protected age."""
        return self._age

    def show(self) -> None:
        """Print a one-line summary of the plant."""
        print(f"{self._name}: {self._height}cm, {self._age} days old")


def main() -> None:
    plant = Plant("Rose", 30, 5)
    plant.set_height(-5)
    plant.set_age(-1)
    plant.set_height(35)
    plant.set_age(6)
    plant.show()
    print(f"Final height: {plant.get_height()}, age: {plant.get_age()}")


if __name__ == "__main__":
    main()
