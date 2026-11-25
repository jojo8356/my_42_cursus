"""Module 2, Exercise 4: Finally Block - Always Clean Up.

Guarantee resource cleanup with a finally block.
"""

import os
import tempfile


def water_plant(filename: str) -> None:
    """Write watering activity to a file and always close it."""
    handle = open(filename, "w")
    try:
        handle.write("watering the plants\n")
    finally:
        handle.close()
        print(f"Watering file closed for {filename}")


def test_watering_system() -> None:
    """Open the watering system, run it, and always clean up."""
    handle_fd, filename = tempfile.mkstemp(suffix=".txt")
    os.close(handle_fd)
    print(f"Opening water system {filename}")
    try:
        water_plant(filename)
        print("Watering complete")
    finally:
        print("Cleanup always happens")
        try:
            os.remove(filename)
        except OSError:
            pass


if __name__ == "__main__":
    test_watering_system()
