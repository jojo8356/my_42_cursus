#!/bin/sh
# Driver C02 : compile les solutions avec la batterie de tests et l'execute.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c02_test_bin"

echo "== C02 =="
if gcc -Wall -Wextra -Werror -o "$OUT" "$ROOT/tests/c02_test.c" \
	"$ROOT"/c02/ex*/*.c 2>"$T/c02_cc.log"; then
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c02_cc.log"
	exit 1
fi
