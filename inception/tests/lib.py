# ============================================================================ #
#  Inception test suite — shared helpers & lightweight test framework.       #
#  Docker is unavailable in this environment, so validation is static:      #
#  YAML/config structure, rule compliance, Dockerfile/config correctness.   #
# ============================================================================ #
import os
import sys

# Resolve the project root (the directory containing this tests/ folder)
TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(TESTS_DIR)
SRC_DIR = os.path.join(PROJECT_ROOT, "srcs")


def read_file(rel_path):
    """Read a file relative to the project root."""
    path = os.path.join(PROJECT_ROOT, rel_path)
    with open(path, "r", encoding="utf-8") as fh:
        return fh.read()


def path_exists(rel_path):
    return os.path.exists(os.path.join(PROJECT_ROOT, rel_path))


# --- Result tracking ---------------------------------------------------------
_RESULTS = {"passed": 0, "failed": 0, "errors": []}


class CheckFailed(Exception):
    pass


def check(condition, label, detail=""):
    """Assert a condition; record pass/fail. Returns the value if truthy."""
    if condition:
        _RESULTS["passed"] += 1
        return condition
    msg = label if not detail else f"{label} — {detail}"
    _RESULTS["failed"] += 1
    _RESULTS["errors"].append(msg)
    raise CheckFailed(msg)


def expect(condition, label, detail=""):
    """Assert without raising (records failure, returns bool)."""
    if condition:
        _RESULTS["passed"] += 1
        return True
    msg = label if not detail else f"{label} — {detail}"
    _RESULTS["failed"] += 1
    _RESULTS["errors"].append(msg)
    return False


def reset_results():
    _RESULTS["passed"] = 0
    _RESULTS["failed"] = 0
    _RESULTS["errors"] = []


def get_results():
    return _RESULTS
