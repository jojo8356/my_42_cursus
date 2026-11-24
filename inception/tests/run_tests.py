#!/usr/bin/env python3
# ============================================================================ #
#  Inception — full test suite runner (digne des plus grandes entreprises)     #
#  Discovers every test_* module in this directory and runs its test_*       #
#  functions. Reports a summary and exits non-zero on any failure.           #
# ============================================================================ #
import os
import sys
import traceback
import importlib
import inspect

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lib

TEST_MODULES = [
    "test_compose",
    "test_dockerfiles",
    "test_nginx",
    "test_wordpress",
    "test_mariadb",
    "test_makefile_env",
    "test_bonus",
]


def color(text, code):
    return f"\033[{code}m{text}\033[0m"


def main():
    lib.reset_results()
    total_checks_before = 0
    passed_before = 0
    failed_before = 0

    print(color("=" * 70, "36"))
    print(color("  INCEPTION — SUITE DE TESTS COMPLÈTE", "36;1"))
    print(color("=" * 70, "36"))
    print(f"  Project root: {lib.PROJECT_ROOT}\n")

    for mod_name in TEST_MODULES:
        try:
            module = importlib.import_module(mod_name)
        except Exception as exc:
            print(color(f"  [MODULE ERROR] {mod_name}: {exc}", "31"))
            lib.get_results()["failed"] += 1
            lib.get_results()["errors"].append(f"module {mod_name} import: {exc}")
            continue

        tests = [
            fn for fn_name, fn in inspect.getmembers(module, inspect.isfunction)
            if fn_name.startswith("test_") and fn.__module__ == mod_name
        ]
        if not tests:
            continue

        print(color(f"── {mod_name} " + "─" * (60 - len(mod_name)), "33"))
        for test in tests:
            name = test.__name__
            passed_before = lib.get_results()["passed"]
            failed_before = lib.get_results()["failed"]
            try:
                test()
                status = color("PASS", "32")
            except lib.CheckFailed as exc:
                status = color("FAIL", "31")
                print(f"    [{status}] {name}: {exc}")
            except Exception as exc:  # unexpected error
                status = color("ERROR", "35")
                print(f"    [{status}] {name}: {exc}")
                traceback.print_exc()
                lib.get_results()["failed"] += 1
                lib.get_results()["errors"].append(f"{name}: unexpected {exc}")
            if lib.get_results()["passed"] > passed_before:
                delta = lib.get_results()["passed"] - passed_before
                print(f"         -> {delta} check(s) OK")
            if lib.get_results()["failed"] > failed_before:
                pass
        print()

    results = lib.get_results()
    total = results["passed"] + results["failed"]
    print(color("=" * 70, "36"))
    print(color("  RÉSULTATS", "36;1"))
    print(color("=" * 70, "36"))
    print(f"  Total checks : {total}")
    print(color(f"  Passés       : {results['passed']}", "32"))
    print(color(f"  Échoués      : {results['failed']}", "31"))
    if results["errors"]:
        print(color("\n  Détail des échecs:", "31;1"))
        for err in results["errors"]:
            print(f"    - {err}")
    print()
    if results["failed"] == 0:
        print(color("  TOUS LES TESTS SONT PASSÉS ✅", "32;1"))
        return 0
    print(color("  DES TESTS ONT ÉCHOUÉ ❌", "31;1"))
    return 1


if __name__ == "__main__":
    sys.exit(main())
