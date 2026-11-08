#!/bin/sh
# Driver C03 : compile les solutions avec la batterie de tests et l'execute.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c03_test_bin"

echo "== C03 =="
if gcc -Wall -Wextra -Werror -o "$OUT" "$ROOT/tests/c03_test.c" \
	"$ROOT"/c03/ex*/*.c 2>"$T/c03_cc.log"; then
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c03_cc.log"
	exit 1
fi
