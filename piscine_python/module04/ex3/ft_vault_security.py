"""Module 4, Exercise 3: Vault Security.

A context-managed archive helper that reads or writes a file safely.
"""


def secure_archive(
    filename: str, action: str | int = "read", content: str = ""
) -> tuple[bool, str]:
    """Read or write a file using a context manager.

    Returns a ``(success, payload)`` tuple: the file content for a successful
    read, a confirmation message for a successful write, or an error message.
    """
    try:
        if action in ("read", 0, 1):
            with open(filename, "r") as handle:
                return (True, handle.read())
        if action in ("write", 2):
            with open(filename, "w") as handle:
                handle.write(content)
            return (True, f"Content written to {filename}")
        return (False, f"Unknown action: {action}")
    except FileNotFoundError:
        return (False, f"File not found: {filename}")
    except OSError as error:
        return (False, str(error))


def main() -> None:
    """Demonstrate reading and writing with secure_archive."""
    result = secure_archive("demo_vault.txt", "write", "secret archive\n")
    print(result)
    read_result = secure_archive("demo_vault.txt", "read")
    print(read_result)


if __name__ == "__main__":
    main()
