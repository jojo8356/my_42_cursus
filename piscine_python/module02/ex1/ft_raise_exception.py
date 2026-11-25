"""Module 2, Exercise 1: Agricultural Data Validation Pipeline.

Explicitly raise exceptions for out-of-range temperatures and recover.
"""


def input_temperature(value: str) -> int:
    """Parse a temperature and raise ValueError when out of range."""
    try:
        temp = int(value)
    except ValueError:
        raise ValueError(f"invalid temperature: {value!r} is not an integer")
    if temp > 50:
        raise ValueError("Temperature too hot: exceeds the maximum limit")
    if temp < 0:
        raise ValueError("Temperature too cold: below the minimum limit")
    return temp


def test_temperature() -> None:
    """Exercise the valid, too-hot and too-cold paths without crashing."""
    try:
        temp = input_temperature("25")
        print(f"Temperature is now {temp}")
    except ValueError as error:
        print(f"error: {error}")

    try:
        input_temperature("100")
    except ValueError as error:
        print(f"error: {error}")

    try:
        input_temperature("-50")
    except ValueError as error:
        print(f"error: {error}")

    print("Program didn't crash")


if __name__ == "__main__":
    test_temperature()
