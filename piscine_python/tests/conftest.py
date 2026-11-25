"""
Helpers partagés pour la suite de tests de la Piscine Python.
Chargement de modules/scripts par chemin (les exercices ne sont pas des
packages importables de manière uniforme).
"""

import importlib.util
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parent.parent


def load_module(rel_path: str, name: str | None = None):
    """Charge un module Python à partir d'un chemin relatif."""
    path = ROOT / rel_path
    name = name or path.stem
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def run_script(rel_path: str, args=None, input_text: str | None = None, cwd: Path | None = None):
    """Exécute un script et renvoie (code, stdout, stderr)."""
    path = ROOT / rel_path
    proc = subprocess.run(
        [sys.executable, str(path), *(args or [])],
        capture_output=True,
        text=True,
        input=input_text,
        cwd=str(cwd or ROOT),
        timeout=30,
    )
    return proc.returncode, proc.stdout, proc.stderr


@pytest.fixture(scope="session")
def module09_ex0():
    return load_module("module09/ex0/space_station.py")


@pytest.fixture(scope="session")
def module09_ex1():
    return load_module("module09/ex1/alien_contact.py")


@pytest.fixture(scope="session")
def module09_ex2():
    return load_module("module09/ex2/space_crew.py")


@pytest.fixture(scope="session")
def module10_ex0():
    return load_module("module10/ex0/lambda_spells.py")


@pytest.fixture(scope="session")
def module10_ex1():
    return load_module("module10/ex1/higher_magic.py")


@pytest.fixture(scope="session")
def module10_ex2():
    return load_module("module10/ex2/scope_mysteries.py")


@pytest.fixture(scope="session")
def module10_ex3():
    return load_module("module10/ex3/functools_artifacts.py")


@pytest.fixture(scope="session")
def module10_ex4():
    return load_module("module10/ex4/decorator_mastery.py")
