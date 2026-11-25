"""Module 2, Exercise 0: Agricultural Data Validation.

Handle an invalid temperature reading without crashing the program.
"""


def input_temperature(value: str) -> int:
    """Parse a temperature and reject values outside the valid range."""
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
    """Demonstrate temperature validation with graceful error handling."""
    try:
        temp = input_temperature("25")
        print(f"Temperature is now {temp}")
        bad = input_temperature("100")
        print(f"Temperature is now {bad}")
    except ValueError as error:
        print(f"error: {error}")
    print("Program didn't crash")


if __name__ == "__main__":
    test_temperature()
