"""Module 3, Exercise 4: Inventory Master.

Parse an item:quantity inventory from command-line arguments and summarise it.
"""

import sys


def main() -> None:
    """Build an inventory from arguments and report statistics."""
    inventory: dict[str, int] = {}
    for arg in sys.argv[1:]:
        if ":" not in arg:
            print(f"Error - invalid parameter '{arg}'")
            continue
        item, _, quantity_str = arg.partition(":")
        try:
            quantity = int(quantity_str)
        except ValueError:
            print(f"Quantity error for '{item}'")
            continue
        if item in inventory:
            print(f"Redundant item '{item}' - discarding")
            continue
        inventory[item] = quantity

    print(f"Got inventory: {inventory}")
    items = list(inventory.keys())
    print(f"Item list: {items}")

    total = sum(inventory.values())
    print(f"Total quantity of the {len(items)} items: {total}")

    most = max(inventory, key=inventory.get)
    print(f"Item most abundant: {most} with quantity {inventory[most]}")

    least = min(inventory, key=inventory.get)
    print(f"Item least abundant: {least} with quantity {inventory[least]}")

    print("Updated inventory:")
    for item, quantity in inventory.items():
        percentage = quantity / total * 100
        print(f"{item} represents {percentage:.1f}% of the inventory")


if __name__ == "__main__":
    main()
