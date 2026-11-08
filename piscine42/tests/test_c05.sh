#!/bin/sh
# Driver C05 : compile les solutions avec la batterie de tests et l'execute.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c05_test_bin"

echo "== C05 =="
if gcc -Wall -Wextra -Werror -o "$OUT" "$ROOT/tests/c05_test.c" \
	"$ROOT"/c05/ex*/*.c 2>"$T/c05_cc.log"; then
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c05_cc.log"
	exit 1
fi
