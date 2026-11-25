"""
Exercice 0 - Entering the Matrix
Détecte si le code tourne dans un environnement virtuel (venv) isolé ou
dans l'environnement global du système.
"""

import os
import sys


def is_virtual_environment() -> bool:
    """Renvoie True si on tourne dans un venv, False sinon.

    La décision repose sur VIRTUAL_ENV (défini par `activate`), pour rester
    cohérent avec la détection d'environnement du runner.
    """
    # VIRTUAL_ENV est défini par activate (venv/conda)
    return bool(os.environ.get("VIRTUAL_ENV"))
    # sys.prefix change également lorsqu'un venv est activé, mais on privilégie
    # la variable d'environnement explicite pour la décision.


def main() -> None:
    print("MATRIX STATUS:")
    print(f"Current Python: {sys.version.split()[0]}")
    print(f"Virtual Environment: {os.environ.get('VIRTUAL_ENV', 'None detected')}")

    if is_virtual_environment():
        print("Welcome to the construct")
        print("SUCCESS: You're in an isolated environment!")
    else:
        print("You're still plugged in")
        print("None detected")
        print("WARNING: You're in the global environment!")
        print("Create an isolated environment first: python -m venv matrix_env")


if __name__ == "__main__":
    main()
