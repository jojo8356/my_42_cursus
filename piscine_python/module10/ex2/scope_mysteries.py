"""
Module 10 - Exercice 2 : Scope Mysteries
Clôtures, nonlocal, fonctions à état et patron de factory fonctionnel.
"""

from typing import Any, Dict

from collections.abc import Callable


def mage_counter() -> Callable:
    """Crée un compteur incrémenté à chaque appel (état isolé par clôture)."""
    called: int = 0

    def count() -> int:
        nonlocal called
        called += 1
        return called

    return count


def spell_accumulator(initial_power: int) -> Callable:
    """Crée une fonction qui accumule la puissance à partir d'une base."""

    def new_total(new_amount: int) -> int:
        nonlocal initial_power
        initial_power += new_amount
        return initial_power

    return new_total


def enchantment_factory(enchantment_type: str) -> Callable:
    """Factory fonctionnelle : renvoie une fonction d'enchantement liée au type."""

    def enchantment(item_name: str) -> str:
        return f"{enchantment_type} {item_name}"

    return enchantment


def memory_vault() -> dict[str, Callable]:
    """Crée un coffre-fort en mémoire avec stockage et rappel."""
    memory: Dict[str, Any] = {}

    def store(key: str, value: Any) -> None:
        memory[key] = value

    def recall(key: str) -> Any:
        try:
            return memory[key]
        except KeyError:
            return "Memory not found"

    return {"store": store, "recall": recall}


def main() -> None:
    counter_a: Callable = mage_counter()
    counter_b: Callable = mage_counter()
    print("Testing mage counter...")
    print(f"counter_a call 1: {counter_a()}")
    print(f"counter_a call 2: {counter_a()}")
    print(f"counter_b call 1: {counter_b()}\n")

    print("Testing spell accumulator......")
    add_accumulator: Callable = spell_accumulator(100)
    print(f"Base 100, add 20: {add_accumulator(20)}")
    print(f"Base 100, add 30: {add_accumulator(30)}")

    print("\nTesting enchantment factory...")
    frozen: Callable = enchantment_factory("Frozen")
    flaming: Callable = enchantment_factory("Flaming")
    print(f"{frozen('Sword')}")
    print(f"{frozen('Shield')}")
    print(f"{flaming('gun')}")

    print("\nTesting memory vault...")
    functions: dict[str, Callable] = memory_vault()
    print("Store 'secret' = 42")
    functions["store"]("secret", 42)
    print(f"Recall 'secret': {functions['recall']('secret')}")
    print(f"Recall 'unknown': {functions['recall']('unknown')}")


if __name__ == "__main__":
    main()
