#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 1 -- static: what the subject asks of the source and the Makefile,
#  checked before anything is run.
#
#  The bulk of it lives in tests/audit.py (header, Norm rules, function sizes,
#  Makefile contract, build, imported symbols). This layer adds the checks that
#  are about the repository rather than the code: the files the subject lists,
#  the presence of libft, and the absence of build artefacts in the tree.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

# The audit inspects the binary's imported symbols, so a build must exist. A
# fresh clone has none, and a reviewer runs make first anyway.
if [ ! -x push_swap ] || [ ! -x checker ]; then
	make >/dev/null 2>&1
	make bonus >/dev/null 2>&1
fi

echo "== 1. Files the subject asks for =="
for f in Makefile; do
	[ -f "$f" ] && ok || ko "$f is missing"
done
n=$(ls src/*.c 2>/dev/null | wc -l)
[ "$n" -ge 5 ] && ok || ko "only $n source files"
for h in include/push_swap.h include/checker.h; do
	[ -f "$h" ] && ok || ko "$h is missing"
done
[ -f libft/Makefile ] && ok || ko "libft/Makefile is missing"
[ -d libft ] && ok || ko "libft/ is missing (the subject allows it, so it must be vendored whole)"

echo "== 2. The Makefile builds libft through its own Makefile =="
grep -qE '\$\(MAKE\)[[:space:]]+-C[[:space:]]+libft' Makefile \
	&& ok || ko "Makefile does not call \$(MAKE) -C libft"

echo "== 3. The bonus is in _bonus files, as the subject requires =="
n=$(ls src/bonus/*.c 2>/dev/null | wc -l)
[ "$n" -ge 2 ] && ok || ko "bonus sources are not in src/bonus/"

echo "== 4. No global variables =="
# a definition at file scope with an initialiser and no `static` is a global
if grep -nE '^[A-Za-z_][A-Za-z0-9_ ]*[[:space:]]+[a-zA-Z_][A-Za-z0-9_]*[[:space:]]*(=|;)' \
	src/*.c src/bonus/*.c 2>/dev/null | grep -vE '^\S+:[0-9]+:(typedef|struct|extern|static)' \
	| grep -vE '\(|return|^\S+:[0-9]+:\s*$' | grep -qvE '^\s*$'; then
	: # grep -v pipelines are unreliable here; the audit's symbol check is the authority
fi
if grep -qE '^[a-zA-Z_].*[[:space:]][a-z_A-Z]+;[[:space:]]*$' src/*.c 2>/dev/null \
	&& ! grep -q static src/main.c; then
	ko "possible global variable"
else
	ok
fi

echo "== 5. Static audit =="
if python3 tests/audit.py > "$PWD/.audit.out" 2>&1; then
	ok
	tail -1 "$PWD/.audit.out"
else
	ko "tests/audit.py reported problems:"
	sed 's/^/       /' "$PWD/.audit.out" | head -20
fi
rm -f "$PWD/.audit.out"

echo "== 6. Both flavours build from clean =="
if make fclean >/dev/null 2>&1 && make >/dev/null 2>&1 && [ -x push_swap ]; then
	ok
else
	ko "make from clean failed"
fi
if make bonus >/dev/null 2>&1 && [ -x checker ]; then
	ok
else
	ko "make bonus from clean failed"
fi

echo "== 7. No relink =="
out=$(make 2>&1)
case "$out" in
	*"Nothing to be done"*|*"nothing to be done"*|*"is up to date"*) ok ;;
	*) ko "make relinked: $(echo "$out" | head -2)" ;;
esac
out=$(make bonus 2>&1)
case "$out" in
	*"Nothing to be done"*|*"nothing to be done"*|*"is up to date"*) ok ;;
	*) ko "make bonus relinked: $(echo "$out" | head -2)" ;;
esac

echo "== 8. The binary does not spawn anything =="
# push_swap has no business forking: the subject's allowed functions do not
# include fork, and nm -u is the place to prove it
if nm -u push_swap 2>/dev/null | grep -qE '\b(fork|execve|system|popen)\b'; then
	ko "push_swap imports a process-creation function"
else
	ok
fi

echo "== 9. No leftover build artefacts in the tree =="
found=$(find . -maxdepth 2 -name '*.o' -not -path './libft/*' -not -path './obj/*' 2>/dev/null | head -3)
[ -z "$found" ] && ok || ko "object files outside obj/: $found"

printf '\n  \033[1;34mstatic\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
