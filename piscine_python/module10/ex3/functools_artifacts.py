"""
Module 10 - Exercice 3 : Functools Artifacts
Le module functools : reduce, partial, lru_cache, singledispatch.
"""

import functools
import operator
from collections.abc import Callable
from typing import Any


def spell_reducer(spells: list[int], operation: str) -> int:
    """Réduit une liste de sorts selon une opération (add/multiply/max/min)."""
    if len(spells) == 0:
        return 0
    match operation:
        case "add":
            return functools.reduce(operator.add, spells)
        case "multiply":
            return functools.reduce(operator.mul, spells)
        case "max":
            return functools.reduce(max, spells)
        case "min":
            return functools.reduce(min, spells)
    raise ValueError(f"{operation} operation is not supported")


def enchantment(power: int, element: str, target: str) -> str:
    return f"{target} got an attack of {power} points with a {element}"


def partial_enchanter(base_enchantment: Callable) -> dict[str, Callable]:
    """Crée une version partielle d'un enchantement (power et element fixés)."""
    return {"enchantment": functools.partial(base_enchantment, 50, "fire")}


@functools.lru_cache(maxsize=128)
def memoized_fibonacci(n: int) -> int:
    """Nombre de Fibonacci mémorisé (lru_cache)."""
    if n <= 1:
        return n
    return memoized_fibonacci(n - 1) + memoized_fibonacci(n - 2)


@functools.singledispatch
def spell_dispatcher(spell: Any) -> Callable[[Any], str]:
    """Dispatch unique selon le type du sort."""
    raise ValueError("Unknown spell type")


@spell_dispatcher.register(int)
def _(spell):
    return lambda x: f"{spell} damage"


@spell_dispatcher.register(str)
def _(spell):
    return lambda x: spell


@spell_dispatcher.register(list)
def _(spell):
    return lambda x: f"{len(spell)} spells"


def main() -> None:
    print("Testing spell reducer...")
    print(f"Sum: {spell_reducer([10, 30, 30, 20, 5, 5], 'add')}")
    print(f"Product: {spell_reducer([60, 20, 200], 'multiply')}")
    print(f"Max: {spell_reducer([30, 20, 40], 'max')}")

    print("\nTesting partial enchanter...")
    enchantment1: dict[str, Callable] = partial_enchanter(enchantment)
    enchantment2: dict[str, Callable] = partial_enchanter(enchantment)
    enchantment3: dict[str, Callable] = partial_enchanter(enchantment)
    print(enchantment1["enchantment"]("Dragon"))
    print(enchantment2["enchantment"]("Warrior"))
    print(enchantment3["enchantment"]("Orc"))

    print("\nTesting memoized fibonacci...")
    print(f"Fib(0): {memoized_fibonacci(0)}")
    print(f"Fib(1): {memoized_fibonacci(1)}")
    print(f"Fib(10): {memoized_fibonacci(10)}")
    print(f"Fib(15): {memoized_fibonacci(15)}")

    print("\nTesting spell dispatcher...")
    integer: Callable = spell_dispatcher(42)
    string: Callable = spell_dispatcher("fireball")
    lists: Callable = spell_dispatcher(["fireball", "blizzard", "tsunami"])
    print(f"Damage spell: {integer('accomplishing with the subject')}")
    print(f"Enchantment: {string('accomplishing with the subject')}")
    print(f"Multi-cast: {lists('accomplishing with the subject')}")
    try:
        spell_dispatcher({})
    except ValueError as e:
        print(e)


if __name__ == "__main__":
    main()
