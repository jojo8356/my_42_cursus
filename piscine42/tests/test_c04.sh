#!/bin/sh
# Driver C04 : compile les solutions avec la batterie de tests et l'execute.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c04_test_bin"

echo "== C04 =="
if gcc -Wall -Wextra -Werror -o "$OUT" "$ROOT/tests/c04_test.c" \
	"$ROOT"/c04/ex*/*.c 2>"$T/c04_cc.log"; then
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c04_cc.log"
	exit 1
fi
