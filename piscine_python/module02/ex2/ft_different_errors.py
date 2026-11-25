"""Module 2, Exercise 2: Different Types of Problems.

Catch and report several distinct built-in exception types.
"""


def garden_operations(total: int, workers: int) -> float:
    """Split a total harvest evenly among workers."""
    return total / workers


def test_error_types() -> None:
    """Demonstrate catching ValueError, ZeroDivisionError, FileNotFoundError and TypeError."""
    try:
        int("abc")
    except ValueError as error:
        print(f"ValueError: {error}")

    try:
        garden_operations(10, 0)
    except ZeroDivisionError as error:
        print(f"ZeroDivisionError: {error}")

    try:
        with open("missing_file.txt", "r") as handle:
            handle.read()
    except FileNotFoundError as error:
        print(f"FileNotFoundError: {error}")

    try:
        "a" + 1
    except TypeError as error:
        print(f"TypeError: {error}")

    print("All error types handled successfully")


if __name__ == "__main__":
    test_error_types()
