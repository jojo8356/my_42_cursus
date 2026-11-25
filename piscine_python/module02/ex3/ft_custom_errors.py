"""Module 2, Exercise 3: Making Your Own Error Types.

Define a custom exception hierarchy for the garden domain.
"""


class GardenError(Exception):
    """Base class for all garden-related errors."""


class PlantError(GardenError):
    """Raised when a plant is in a bad state."""


class WaterError(GardenError):
    """Raised when the watering system fails."""


def main() -> None:
    try:
        raise PlantError("A plant is sick")
    except PlantError as error:
        print(f"PlantError: {error}")

    try:
        raise WaterError("Water supply is empty")
    except WaterError as error:
        print(f"WaterError: {error}")

    try:
        raise GardenError("General garden failure")
    except GardenError as error:
        print(f"GardenError: {error}")


if __name__ == "__main__":
    main()
