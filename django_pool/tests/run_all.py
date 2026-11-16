#!/usr/bin/env python3
"""Execute tous les tests du harnais django_pool/tests/.

Usage :
    python3 django_pool/tests/run_all.py
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent

CHECKS = [
    "check_d00.py",
    "check_d01.py",
    "check_d02.py",
    "check_d03.py",
    "check_d04.py",
    "check_d05.py",
    "check_d06.py",
    "check_d07.py",
    "check_d08.py",
]


def main() -> int:
    failures = []
    for name in CHECKS:
        print(f"\n{'=' * 70}\n{name}\n{'=' * 70}")
        proc = subprocess.run([sys.executable, str(HERE / name)],
                              cwd=str(HERE))
        if proc.returncode != 0:
            failures.append(name)

    print(f"\n{'=' * 70}")
    if failures:
        print("Harnais en echec : " + ", ".join(failures))
        return 1
    print("Harnais complet : tous les jours passent.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
