"""Module 0, Exercise 1: Garden Name.

Read the garden name from standard input and report its status.
"""


def ft_garden_name() -> None:
    """Read the garden name and print a summary with its status."""
    name = input().strip()
    print(f"Garden: {name}")
    print("Status: Growing well!")


if __name__ == "__main__":
    ft_garden_name()
