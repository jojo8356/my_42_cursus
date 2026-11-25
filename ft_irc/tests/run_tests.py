#!/usr/bin/env python3
# ============================================================================ #
#  ft_irc — full test suite runner (static + integration)                    #
# ============================================================================ #
import os
import sys
import traceback
import importlib
import inspect

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lib

TEST_MODULES = [
    "test_static",
    "test_integration",
]


def color(text, code):
    return f"\033[{code}m{text}\033[0m"


def main():
    lib.reset_results()
    print(color("=" * 70, "36"))
    print(color("  FT_IRC — SUITE DE TESTS COMPLÈTE (statique + intégration)", "36;1"))
    print(color("=" * 70, "36"))
    print(f"  Project root: {lib.PROJECT_ROOT}\n")

    for mod_name in TEST_MODULES:
        try:
            module = importlib.import_module(mod_name)
        except Exception as exc:
            print(color(f"  [MODULE ERROR] {mod_name}: {exc}", "31"))
            lib.get_results()["failed"] += 1
            lib.get_results()["errors"].append(f"module {mod_name}: {exc}")
            continue

        tests = [
            fn for fn_name, fn in inspect.getmembers(module, inspect.isfunction)
            if fn_name.startswith("test_") and fn.__module__ == mod_name
        ]
        if not tests:
            continue

        print(color(f"── {mod_name} " + "─" * (58 - len(mod_name)), "33"))
        for test in tests:
            name = test.__name__
            passed_before = lib.get_results()["passed"]
            try:
                test()
                delta = lib.get_results()["passed"] - passed_before
                print(f"    [\033[32mPASS\033[0m] {name}" +
                      (f"  ({delta} check(s))" if delta else ""))
            except lib.CheckFailed as exc:
                print(f"    [\033[31mFAIL\033[0m] {name}: {exc}")
            except Exception as exc:
                print(f"    [\033[35mERROR\033[0m] {name}: {exc}")
                traceback.print_exc()
                lib.get_results()["failed"] += 1
                lib.get_results()["errors"].append(f"{name}: unexpected {exc}")
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
