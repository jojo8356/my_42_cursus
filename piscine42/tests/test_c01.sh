#!/bin/sh
# Driver C01 : compile les solutions avec la batterie de tests et l'execute.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c01_test_bin"

echo "== C01 =="
if gcc -Wall -Wextra -Werror -o "$OUT" "$ROOT/tests/c01_test.c" \
	"$ROOT"/c01/ex*/*.c 2>"$T/c01_cc.log"; then
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c01_cc.log"
	exit 1
fi
