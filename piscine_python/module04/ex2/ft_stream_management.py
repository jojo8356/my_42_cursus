"""Module 4, Exercise 2: Stream Management.

Manage file streams using sys.stdin/stdout/stderr and readline().
"""

import sys


def main() -> None:
    """Copy a file's content to a destination, reporting errors on stderr."""
    filename = sys.argv[1] if len(sys.argv) > 1 else ""

    try:
        handle = open(filename, "r")
        content = handle.read()
        handle.close()
    except FileNotFoundError as error:
        print(f"[STDERR] Error: {error}", file=sys.stderr)
        return

    save_name = sys.stdin.readline().strip()
    try:
        out = open(save_name, "w")
        out.write(content)
        out.close()
        print("Data saved.")
    except (PermissionError, OSError) as error:
        print(f"[STDERR] Error: {error}", file=sys.stderr)
        print("Data not saved.")


if __name__ == "__main__":
    main()
