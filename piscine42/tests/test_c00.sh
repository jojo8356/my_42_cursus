#!/bin/sh
# Driver C00 : compile les solutions avec la batterie de tests et l'execute.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c00_test_bin"

echo "== C00 =="
if gcc -Wall -Wextra -Werror -o "$OUT" "$ROOT/tests/c00_test.c" \
	"$ROOT"/c00/ex*/*.c 2>"$T/c00_cc.log"; then
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c00_cc.log"
	exit 1
fi
