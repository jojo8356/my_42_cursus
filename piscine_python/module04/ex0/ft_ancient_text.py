"""Module 4, Exercise 0: Ancient Text Recovery.

Read a file with an explicit close and report errors gracefully.
"""

import sys


def main() -> None:
    """Open, read and explicitly close the requested archive file."""
    filename = sys.argv[1] if len(sys.argv) > 1 else ""
    print("=== Cyber Archives Recovery ===")
    handle = None
    try:
        handle = open(filename, "r")
        print(f"Accessing file '{filename}'")
        print("---")
        print(handle.read().rstrip("\n"))
    except FileNotFoundError as error:
        print(f"Error opening file '{filename}'")
        print(error)
    finally:
        if handle is not None:
            handle.close()
            print(f"File '{filename}' closed.")


if __name__ == "__main__":
    main()
