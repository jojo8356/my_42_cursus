"""Module 4, Exercise 1: Archive Creation.

Archive a source file's lines by appending a marker, saving to a user name.
"""

import sys


def main() -> None:
    """Read a source file and archive its lines to a chosen destination."""
    source = sys.argv[1] if len(sys.argv) > 1 else ""
    handle = open(source, "r")
    content = handle.read()
    handle.close()

    save_name = input().strip()
    if not save_name:
        print("Not saving data.")
        return

    print(f"Saving data to {save_name}")
    out = open(save_name, "w")
    for line in content.splitlines():
        out.write(f"{line}#\n")
    out.close()
    print(f"Data saved in file {save_name}")


if __name__ == "__main__":
    main()
