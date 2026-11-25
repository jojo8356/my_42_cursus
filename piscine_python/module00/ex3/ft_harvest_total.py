"""Module 0, Exercise 3: Harvest Total.

Sum the harvest of three days and print the total.
"""


def ft_harvest_total() -> None:
    """Read three daily harvest values and print their total."""
    day1 = int(input().strip())
    day2 = int(input().strip())
    day3 = int(input().strip())
    print(f"Total harvest: {day1 + day2 + day3}")


if __name__ == "__main__":
    ft_harvest_total()
