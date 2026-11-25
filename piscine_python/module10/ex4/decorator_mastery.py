"""
Module 10 - Exercice 4 : Decorator Mastery
Décorateurs simples et paramétrés, @functools.wraps, timer, validator, retry.
"""

import functools
import time
from collections.abc import Callable
from typing import Any, Union


def spell_timer(func: Callable) -> Callable:
    """Décorateur qui mesure et affiche le temps d'exécution d'un sort."""

    @functools.wraps(func)
    def measures_execution(*args: tuple, **kwargs: dict) -> Any:
        start: float = time.time()
        result: Any = func(*args, **kwargs)
        time.sleep(0.5)
        end: float = time.time()
        print(f"Spell completed in {end - start:.3f} seconds after execution")
        return result

    return measures_execution


@spell_timer
def heal(target: str, attack: int) -> str:
    return f"Heals {target}"


def power_validator(min_power: int) -> Callable:
    """Décorateur paramétré : valide la puissance avant d'exécuter."""

    def wrapper(func: Callable) -> Callable:
        @functools.wraps(func)
        def validator(*args: tuple, **kwargs: dict) -> Union[Any, str]:
            try:
                if kwargs["power"] >= min_power:
                    return func(*args, **kwargs)
                return "Insufficient power for this spell"
            except KeyError:
                try:
                    if args[0] >= min_power:
                        return func(*args, **kwargs)
                    return "Insufficient power for this spell"
                except IndexError as e:
                    return f"Not arguments given {e}"
                except Exception as e:
                    return f"Something went wrong {e}"

        return validator

    return wrapper


@power_validator(50)
def testing_power(power: int, name: str) -> str:
    return f"{name} attacked with {power} points"


def retry_spell(max_attempts: int) -> Callable:
    """Décorateur paramétré : réessaie un sort en cas d'échec."""

    def wrapper(func: Callable) -> Callable:
        @functools.wraps(func)
        def retrier(*args: tuple, **kwargs: dict) -> Any:
            counter: int = 1
            while counter < max_attempts:
                try:
                    return func(*args, **kwargs)
                except Exception:
                    counter += 1
                    print(f"Spell failed, retrying... (attempt {counter}/{max_attempts})")
            return "Spell casting failed after max_attempts attempts"

        return retrier

    return wrapper


@retry_spell(5)
def testing_retrier(state: list) -> str:
    state[0] += 1
    if state[0] < 3:
        raise Exception
    return "Waaaaaaagh spelled"


class MageGuild:
    """Guilde de mages illustrant les décorateurs en méthode."""

    @staticmethod
    def validate_mage_name(name: str) -> bool:
        if len(name) >= 3 and all(letter.isalpha() or letter.isspace() for letter in name):
            return True
        return False

    @power_validator(10)
    def cast_spell(self, spell_name: str, power: int) -> str:
        return f"Successfully cast {spell_name} with {power} power"


def main() -> None:
    print(f"Casting {heal.__name__}...")
    heal("Warrior", 500)

    print(f"\nExecuting {testing_power.__name__}...")
    print(f"Good: {testing_power(55, 'Sword')}")
    print(f"Error: {testing_power(5, 'Sword')}")

    print(f"\nExecuting {testing_retrier.__name__}...")
    state: list[int] = [0]
    print(testing_retrier(state))

    print("\nTesting MageGuild...")
    mageguild1: MageGuild = MageGuild()
    print(mageguild1.validate_mage_name("Mago de oz"))
    print(mageguild1.validate_mage_name("3Mago de oz"))
    print(mageguild1.cast_spell(spell_name="Expelliarmus", power=20))
    print(mageguild1.cast_spell(spell_name="Lumos", power=9))


if __name__ == "__main__":
    main()
