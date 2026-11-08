#!/usr/bin/env bash
# Full verification orchestrator for get_next_line (Subject v14.3 — Mandatory + Bonus).
#   ./run_tests.sh          -> Norm + Symbol/Static/README audit + Unit/ASan + Tripouille + fsoares + gnl_station
#   ./run_tests.sh --strict -> Also runs Francinette fsoares STRICT_MEM (exhaustive malloc-fail + 1000 FDs)
set -euo pipefail
cd "$(dirname "$0")"

STRICT=0
[ "${1:-}" = "--strict" ] && STRICT=1

echo "============================================================"
echo " 1. 42 Norm v4.1, Allowed Symbols, Static Var & v14.3 Audit"
echo "============================================================"
if command -v norminette >/dev/null 2>&1; then
    norminette get_next_line.c get_next_line_utils.c get_next_line.h \
        get_next_line_bonus.c get_next_line_utils_bonus.c get_next_line_bonus.h
else
    echo "norminette binary not installed -> running built-in 42 Norm v4.1 static verifier..."
fi

python3 - << 'PY'
import glob, re, subprocess, sys

files = [
    "get_next_line.h", "get_next_line.c", "get_next_line_utils.c",
    "get_next_line_bonus.h", "get_next_line_bonus.c", "get_next_line_utils_bonus.c",
]
issues = []

for path in files:
    with open(path, encoding="utf-8") as f:
        lines = f.readlines()
    if len(lines) < 11 or not lines[0].startswith("/* ***"):
        issues.append(f"{path}: missing 42 header")
    else:
        for i in range(11):
            if len(lines[i].rstrip("\n")) != 80:
                issues.append(f"{path}:{i+1}: header line length != 80")
    in_comment = False
    fn_count = 0
    depth = 0
    fn_start = None
    for idx, raw in enumerate(lines, 1):
        expanded = raw.rstrip("\n").expandtabs(4)
        if len(expanded) > 80:
            issues.append(f"{path}:{idx}: line > 80 cols ({len(expanded)})")
        s = raw.strip()
        if s.startswith("/*") and "*/" not in s[2:]:
            in_comment = True
            continue
        if in_comment:
            if "*/" in s:
                in_comment = False
            continue
        if s.startswith(("/*", "*", "//", "#")):
            continue
        clean = re.sub(r'"(\\.|[^"\\])*"', '""', raw)
        clean = re.sub(r"'(\\.|[^'\\])*'", "''", clean)
        clean = re.sub(r'/\*.*?\*/', '', clean)
        if "?" in clean:
            issues.append(f"{path}:{idx}: forbidden ternary '?'")
        if re.search(r'\b(for|switch|goto)\b', clean):
            issues.append(f"{path}:{idx}: forbidden control keyword")
        if path.endswith(".c"):
            if depth == 0 and "{" in clean:
                fn_count += 1
                fn_start = idx
            depth += clean.count("{") - clean.count("}")
            if depth == 0 and fn_start is not None:
                body = idx - fn_start - 1
                if body > 25:
                    issues.append(f"{path}:{fn_start}: function body {body} lines (>25)")
                fn_start = None
    if path.endswith(".c") and fn_count > 5:
        issues.append(f"{path}: {fn_count} functions (>5)")

# Verify static variable count == 1 in bonus & mandatory
m_static = sum(1 for fn in ("get_next_line.c", "get_next_line_utils.c")
               for ln in open(fn) if re.search(r'\bstatic\b.*;', ln))
b_static = sum(1 for fn in ("get_next_line_bonus.c", "get_next_line_utils_bonus.c")
               for ln in open(fn) if re.search(r'\bstatic\b.*;', ln))
if m_static != 1:
    issues.append(f"Mandatory static variable count = {m_static} (expected 1)")
if b_static != 1:
    issues.append(f"Bonus static variable count = {b_static} (expected 1)")

# Verify external symbols (only read, malloc, free allowed)
subprocess.run(["cc", "-Wall", "-Wextra", "-Werror", "-c",
                "get_next_line.c", "get_next_line_utils.c",
                "get_next_line_bonus.c", "get_next_line_utils_bonus.c"], check=True)
allowed_undef = {"read", "malloc", "free", "_GLOBAL_OFFSET_TABLE_"}
internal = {"get_next_line", "gnl_clear", "gnl_memcpy", "gnl_find_nl", "gnl_reserve", "gnl_del_node"}
for obj in ("get_next_line.o", "get_next_line_utils.o", "get_next_line_bonus.o", "get_next_line_utils_bonus.o"):
    out = subprocess.check_output(["nm", obj], text=True)
    for line in out.splitlines():
        parts = line.split()
        sym_type = parts[-2] if len(parts) >= 2 else ""
        sym = parts[-1].split("@")[0]
        if sym_type == "U" and sym not in allowed_undef and sym not in internal:
            issues.append(f"{obj}: forbidden external symbol '{sym}'")
        if sym_type in ("B", "D", "C") and not sym.startswith("stash.") and not sym.startswith("head."):
            issues.append(f"{obj}: forbidden global variable '{sym}' ({sym_type})")
subprocess.run(["rm", "-f", "get_next_line.o", "get_next_line_utils.o",
                "get_next_line_bonus.o", "get_next_line_utils_bonus.o"], check=True)

# Verify README.md v14.3 compliance
with open("README.md", encoding="utf-8") as f:
    rd = f.read()
first_line = rd.splitlines()[0] if rd.splitlines() else ""
if not re.match(r'^\*This project has been created as part of the 42 curriculum by .+\.\*$', first_line):
    issues.append(f"README.md line 1 invalid for v14.3: {first_line!r}")
for sec in ("Description", "Instructions", "Algorithm", "Resources"):
    if sec not in rd:
        issues.append(f"README.md missing required section '{sec}'")

if issues:
    for it in issues:
        print("  [KO]", it)
    sys.exit(1)
print(f"  [PASS] 6/6 files Norm-clean | Mandatory static={m_static} | Bonus static={b_static} | Externals={{read,malloc,free}} | README v14.3 OK")
PY

echo
echo "============================================================"
echo " 2. Edge-Case, Pipe, Binary, High-FD, O(L) & ASan/UBSan Suite"
echo "============================================================"
make -C tests all

echo
echo "============================================================"
echo " 3. Tripouille gnlTester (Mandatory + Bonus: BS=1, 42, 10M)"
echo "============================================================"
make -C gnlTester a
make -C gnlTester fclean

echo
echo "============================================================"
echo " 4. Francinette fsoares Tester (Mandatory + Bonus)"
echo "============================================================"
make -C tests/fsoares mandatory bonus clean
if [ "$STRICT" = "1" ]; then
    echo "--- Running Francinette fsoares STRICT_MEM (malloc fault-injection + 1000 FDs) ---"
    make -C tests/fsoares mandatory bonus clean EXEC_STRICT=1
fi

echo
echo "============================================================"
echo " 5. GNL Station Tester (sloquet: BS=0, 1, 2, 42, 42000)"
echo "============================================================"
make -C tests/gnl_station all

echo
echo "============================================================"
echo " ALL GET_NEXT_LINE CHECKS PASSED (100% MANDATORY + BONUS)"
echo "============================================================"
