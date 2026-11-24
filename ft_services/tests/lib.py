# ============================================================================ #
#  ft_services test suite — shared helpers & lightweight test framework.     #
#  Docker/minikube/kubectl are unavailable here, so validation is static:    #
#  shell syntax, Dockerfile rules, YAML manifests, configs, subject rules.   #
# ============================================================================ #
import os
import sys
import subprocess

TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(TESTS_DIR)
SRC_DIR = os.path.join(PROJECT_ROOT, "srcs")
YAML_DIR = os.path.join(PROJECT_ROOT, "yaml")


def read_file(rel_path):
    """Read a file relative to the project root."""
    path = os.path.join(PROJECT_ROOT, rel_path)
    with open(path, "r", encoding="utf-8") as fh:
        return fh.read()


def path_exists(rel_path):
    return os.path.exists(os.path.join(PROJECT_ROOT, rel_path))


def bash_syntax_ok(rel_path):
    """Return (ok, output) for `bash -n` on a shell script."""
    path = os.path.join(PROJECT_ROOT, rel_path)
    res = subprocess.run(["bash", "-n", path], capture_output=True, text=True)
    return res.returncode == 0, res.stderr


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
