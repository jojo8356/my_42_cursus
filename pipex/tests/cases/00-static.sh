#!/usr/bin/env bash
# Static checks: everything that can be proven without running pipex.
# Every check is one line of output and one point in the score.

set -uo pipefail
cd "$(dirname "$0")/../.." || exit 1

PASS=0
FAIL=0
ko()  { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$1"; }
ok()  { PASS=$((PASS + 1)); }
check() { if eval "$2"; then ok; else ko "$1"; fi; }

echo "== 1. Deliverables named by the subject =="
check "pipex binary exists at the root"          '[ -x pipex ]'
check "Makefile exists"                          '[ -f Makefile ]'
check "Makefile has NAME, all, clean, fclean, re" \
      'grep -qE "^all:" Makefile && grep -qE "^clean:" Makefile && grep -qE "^fclean:" Makefile && grep -qE "^re:" Makefile'
check "Makefile has the bonus rule"              'grep -qE "^bonus:" Makefile'
check "Makefile uses cc"                         'grep -qE "^CC[[:space:]]*=" Makefile'
check "Makefile uses -Wall -Wextra -Werror" \
      'grep -q -- "-Wall" Makefile && grep -q -- "-Wextra" Makefile && grep -q -- "-Werror" Makefile'

echo "== 2. Layout of the turn-in files =="
check "headers live in include/"                 'ls include/*.h >/dev/null 2>&1'
check "mandatory part has pipex.h"                '[ -f include/pipex.h ]'
check "bonus part has pipex_bonus.h"              '[ -f include/pipex_bonus.h ]'
check "at most 1 header at the project root"      '[ "$(ls ./*.h 2>/dev/null | wc -l)" -le 1 ]'
check "libft is present and used"                 '[ -f libft/Makefile ] && grep -q libft Makefile'

echo "== 3. 42 headers =="
missing=0
for f in $(find src include -name '*.c' -o -name '*.h' | sort); do
	[ -f "$f" ] || continue
	head -1 "$f" | grep -q '^/\* \*' || { ko "$f: no 42 header"; missing=$((missing + 1)); }
	awk 'NR<=11 && length($0)!=80 {exit 1}' "$f" || { ko "$f: header is not 80 columns"; missing=$((missing + 1)); }
	grep -q 'By:' "$f" || { ko "$f: no author"; missing=$((missing + 1)); }
done
[ "$missing" -eq 0 ] && ok

echo "== 4. Norm basics =="
long=$(grep -rn '.\{81\}' src include 2>/dev/null | grep -v '^Binary' | wc -l)
check "no line longer than 80 columns ($long found)" '[ "$long" -eq 0 ]'
spaces=$(grep -rn '^    ' src include 2>/dev/null | wc -l)
check "indentation uses tabs ($spaces space-indented lines)" '[ "$spaces" -eq 0 ]'

echo "== 5. Static audit (functions, symbols, rules) =="
if python3 tests/audit.py --no-relink >/tmp/pipex-audit.log 2>&1; then
	ok
else
	ko "tests/audit.py reported problems:"
	sed 's/^/       /' /tmp/pipex-audit.log | head -10
fi

echo "== 6. Both flavours build =="
make fclean >/dev/null 2>&1
if make >/dev/null 2>&1 && [ -x pipex ]; then ok; else ko "make (mandatory) failed"; fi
if make bonus >/dev/null 2>&1 && [ -x pipex ]; then ok; else ko "make bonus failed"; fi
check "make bonus rebuilt the binary from bonus objects" 'nm pipex | grep -q px_run_bonus'
make fclean >/dev/null 2>&1 && make bonus >/dev/null 2>&1

printf '\n  \033[1;34mstatic\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
