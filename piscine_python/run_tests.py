#!/usr/bin/env python3
"""
Runner de tests unifié - Piscine Python.
Exécute la suite de tests maison (structure + modules 09/10 + régression)
via pytest et affiche un récapitulatif.
"""

import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parent
TESTS = ROOT / "tests"


def main() -> int:
    print("=" * 64)
    print("PISCINE PYTHON - SUITE DE TESTS")
    print("=" * 64)
    args = [str(TESTS), "-v", "--tb=short"]
    return pytest.main(args)


if __name__ == "__main__":
    sys.exit(main())
