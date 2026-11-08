#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 7 -- the community tester, unmodified.
#
#  augustobecker/so_long_tester is kept verbatim under
#  tests/community/augustobecker/, including its 13 check scripts and its 41
#  maps. Its scripts are written to be executed from inside tests/, and run the
#  program as `./../so_long`, piping it through grep to decide whether a map was
#  rejected. Nothing here patches them.
#
#  Two things are needed to run them on this machine:
#
#   1. The binary has to sit exactly where the scripts look for it, one level
#      above tests/. That is why the tree is staged in a temporary directory
#      instead of being run in place: the repository keeps its own layout, and
#      the upstream files stay byte-for-byte identical.
#
#   2. There is no X server here, so the binary staged is the headless build
#      linked against tests/shim, not a real minilibX one.
#
#  The upstream Makefile also has a "make -C ../" step, which would rebuild the
#  real project against a minilibX that is not installed. The check scripts are
#  therefore invoked directly, in the Makefile's own order.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
STATE=${SOLONG_STATE:-/tmp/solong_ci}
SRC="$ROOT/tests/community/augustobecker"
BIN="$STATE/so_long"

[ -x "$BIN" ] || { echo "binary missing: run tests/build.sh"; exit 1; }
[ -d "$SRC" ] || { echo "community tester missing: $SRC"; exit 1; }

STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT

#   <stage>/so_long        the binary, reached as ./../so_long from tests/
#   <stage>/tests/*.sh     the 13 checks, executed with tests/ as the cwd
#   <stage>/maps/...       the corpus, addressed as maps/invalid/...
cp -a "$SRC"/. "$STAGE/"
cp "$BIN" "$STAGE/so_long"
chmod +x "$STAGE/so_long" "$STAGE"/tests/*.sh

# valgrind is not installed and cannot be, so four of the thirteen checks would
# fail before reaching their real assertion. The shim in tests/tools/valgrind
# makes them runnable; it does not check memory. tests/cases/40-memory.sh does
# that for real, with ASan and UBSan, on this same corpus. The substitution is
# announced on stdout so a green run is never mistaken for a leak check.
mkdir -p "$STAGE/bin"
cp "$ROOT/tests/tools/valgrind" "$STAGE/bin/valgrind"
chmod +x "$STAGE/bin/valgrind"
export PATH="$STAGE/bin:$PATH"
printf 'note: valgrind is replaced by tests/tools/valgrind (no leak checking);\n'
printf '      memory is checked by tests/cases/40-memory.sh with ASan+UBSan.\n\n'


# The 13 checks, in the order the upstream Makefile lists them.
CHECKS="check-for-no-map.sh
check-for-missing-argv.sh
check-for-multiple-argv.sh
check-for-invalid-extension.sh
check-for-empty-line.sh
check-for-not-rectangular-map.sh
check-for-invalid-parameter.sh
check-for-missing-W.sh
check-for-missing-C.sh
check-for-missing-E.sh
check-for-missing-P.sh
check-for-extra-P.sh
check-for-extra-E.sh"

OK=0
KO=0
printf 'staging: %s\n\n' "$STAGE"
for check in $CHECKS; do
	[ -f "$STAGE/tests/$check" ] || { printf 'missing check: %s\n' "$check"; \
		KO=$((KO + 1)); continue; }
	# run from tests/, exactly as the upstream Makefile does
	out=$(cd "$STAGE/tests" && ./"$check" 2>&1)
	printf '%s\n' "$out"
	OK=$((OK + $(printf '%s\n' "$out" | grep -o '\[OK\]' | wc -l)))
	KO=$((KO + $(printf '%s\n' "$out" | grep -o '\[KO\]' | wc -l)))
done

printf '\n  \033[1;34mcommunity\033[0m: %d [OK], %d [KO]\n' "$OK" "$KO"

if [ "$KO" -ne 0 ]; then
	printf '  \033[31mthe third-party tester reports failures\033[0m\n'
	exit 1
fi
if [ "$OK" -eq 0 ]; then
	printf '  \033[31mno check produced a verdict\033[0m\n'
	exit 1
fi
exit 0
