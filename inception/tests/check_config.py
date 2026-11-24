#!/usr/bin/env python3
# ============================================================================ #
#  Inception — quick configuration validator (Makefile 'check' target)       #
#  Validates YAML syntax of the compose file and reports OK/FAIL.            #
# ============================================================================ #
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tests"))

import yaml

TARGETS = ["srcs/docker-compose.yml"]

ok = True
for rel in TARGETS:
    path = os.path.join(ROOT, rel)
    try:
        with open(path, "r", encoding="utf-8") as fh:
            yaml.safe_load(fh)
        print(f"  [OK] {rel}: YAML valide")
    except Exception as exc:
        print(f"  [FAIL] {rel}: {exc}")
        ok = False

sys.exit(0 if ok else 1)
