"""Module 0, Exercise 2: Garden Plot Area.

Compute the area of a rectangular garden plot from its dimensions.
"""


def ft_plot_area() -> None:
    """Read length and width, then print the resulting plot area."""
    length = int(input().strip())
    width = int(input().strip())
    print(f"Plot area: {length * width}")


if __name__ == "__main__":
    ft_plot_area()
