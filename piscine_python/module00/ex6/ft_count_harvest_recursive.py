"""Module 0, Exercise 6: Count to Harvest (recursive).

Count the days leading up to harvest using recursion.
"""


def ft_count_harvest_recursive(day: int = 1, total: int | None = None) -> None:
    """Count each day recursively until the harvest day is reached.

    When called with no arguments, the total number of days is read from
    standard input.
    """
    if total is None:
        total = int(input().strip())
    if day > total:
        print("Harvest time!")
        return
    print(f"Day {day}")
    ft_count_harvest_recursive(day + 1, total)


if __name__ == "__main__":
    ft_count_harvest_recursive()
