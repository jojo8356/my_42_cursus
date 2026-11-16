"""Petit harnais commun pour les tests du django_pool.

Chaque `check_dXX.py` importe ce module, enchaine des verifier() et
affiche un resume en fin d'execution.  Le lanceur `run_tests.sh` execute
tous les checkers et aggrege leurs codes de retour.
"""

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

TESTS_DIR = Path(__file__).resolve().parent
POOL_DIR = TESTS_DIR.parent
FIXTURES_DIR = TESTS_DIR / "fixtures"


class Report:
    def __init__(self, name: str) -> None:
        self.name = name
        self.passed = 0
        self.failed = 0
        self.skipped = 0
        self.lines: list[str] = []

    def ok(self, label: str) -> None:
        self.passed += 1
        self.lines.append(f"  PASS  {label}")

    def fail(self, label: str, detail: str = "") -> None:
        self.failed += 1
        self.lines.append(f"  FAIL  {label}" + (f"\n        {detail}" if detail else ""))

    def skip(self, label: str, why: str = "") -> None:
        self.skipped += 1
        self.lines.append(f"  SKIP  {label}" + (f" ({why})" if why else ""))

    def check(self, label: str, cond: bool, detail: str = "") -> bool:
        self.ok(label) if cond else self.fail(label, detail)
        return cond

    def equals(self, label: str, got, want) -> bool:
        return self.check(label, got == want, f"attendu: {want!r}\n        obtenu: {got!r}")

    def summary(self) -> int:
        total = self.passed + self.failed + self.skipped
        print(f"\n=== {self.name} ===")
        print("\n".join(self.lines))
        print(
            f"  -> {self.passed}/{total} reussis, "
            f"{self.failed} echecs, {self.skipped} ignores"
        )
        return 1 if self.failed else 0


def run(
    cmd,
    cwd: Path | str | None = None,
    env: dict | None = None,
    timeout: float = 60,
    stdin=None,
):
    """Execute une commande et retourne (returncode, stdout, stderr)."""
    full_env = dict(os.environ)
    if env:
        full_env.update(env)
    proc = subprocess.run(
        cmd,
        cwd=str(cwd) if cwd else None,
        env=full_env,
        input=stdin,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    return proc.returncode, proc.stdout, proc.stderr


def make_tmp(prefix: str = "djtest-") -> Path:
    return Path(tempfile.mkdtemp(prefix=prefix))


def stage(files: dict, base: Path | None = None) -> Path:
    """Copie {destination: source} dans un dossier temporaire et le retourne."""
    tmp = base or make_tmp()
    for dest, src in files.items():
        target = tmp / dest
        target.parent.mkdir(parents=True, exist_ok=True)
        if isinstance(src, (list, tuple)):
            for s in src:
                shutil.copy(s, target)
        else:
            shutil.copy(src, target)
    return tmp


def run_py(script: Path, args=(), cwd: Path | None = None, timeout=60):
    return run([sys.executable, str(script), *map(str, args)], cwd=cwd, timeout=timeout)


def read(path: Path) -> str:
    return Path(path).read_text(encoding="utf-8", errors="replace")


def norm(text: str) -> str:
    return text.strip().replace("\r\n", "\n")


def dedent(text: str) -> str:
    return text.replace("\t", "    ")


def assert_contains(report: Report, label: str, haystack: str, needle: str) -> bool:
    return report.check(
        label,
        needle in haystack,
        f"{needle!r} absent de {haystack[:400]!r}",
    )


def assert_regex(report: Report, label: str, haystack: str, pattern: str) -> bool:
    return report.check(
        label,
        re.search(pattern, haystack, re.MULTILINE) is not None,
        f"/{pattern}/ ne correspond pas dans {haystack[:400]!r}",
    )
