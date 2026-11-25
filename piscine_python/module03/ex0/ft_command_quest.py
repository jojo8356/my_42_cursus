"""Module 3, Exercise 0: Command Quest.

Explore command-line arguments with sys.argv.
"""

import sys


def main() -> None:
    """Report the received arguments and the total argument count."""
    args = sys.argv[1:]
    print("=== Command Quest ===")
    if not args:
        print("No arguments provided!")
    else:
        print(f"Arguments received: {len(args)}")
        for index, arg in enumerate(args, 1):
            print(f"Argument {index}: {arg}")
    print(f"Total arguments: {len(sys.argv)}")


if __name__ == "__main__":
    main()
