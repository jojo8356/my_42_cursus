"""Module 0, Exercise 5: Water Reminder.

Decide whether the plants need watering based on a soil-moisture reading.
"""


def ft_water_reminder() -> None:
    """Read the moisture level and remind whether to water the plants."""
    moisture = int(input().strip())
    if moisture <= 2:
        print("Plants are fine")
    else:
        print("Water the plants!")


if __name__ == "__main__":
    ft_water_reminder()
