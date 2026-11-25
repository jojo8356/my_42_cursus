"""Module 0, Exercise 6: Count to Harvest (iterative).

Count the days leading up to harvest using a loop.
"""


def ft_count_harvest_iterative() -> None:
    """Read the number of days and count each day until harvest."""
    total = int(input().strip())
    for day in range(1, total + 1):
        print(f"Day {day}")
    print("Harvest time!")


if __name__ == "__main__":
    ft_count_harvest_iterative()
