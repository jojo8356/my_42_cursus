"""Helpers partagés pour la suite de tests d'A-Maze-ing."""

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PY = sys.executable
sys.path.insert(0, str(ROOT))


def run_cli(args, input_text: str | None = None):
    """Exécute a_maze_ing.py et renvoie (code, stdout, stderr)."""
    proc = subprocess.run(
        [PY, str(ROOT / "a_maze_ing.py"), *args],
        capture_output=True,
        text=True,
        input=input_text,
        cwd=str(ROOT),
        timeout=60,
    )
    return proc.returncode, proc.stdout, proc.stderr
