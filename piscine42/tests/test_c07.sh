#!/bin/sh
# Driver C07 : compile les solutions avec la batterie de tests (valgrind
# si disponible) et l'execute.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c07_test_bin"

echo "== C07 =="
if gcc -Wall -Wextra -Werror -o "$OUT" "$ROOT/tests/c07_test.c" \
	"$ROOT"/c07/ex*/*.c 2>"$T/c07_cc.log"; then
	if command -v valgrind >/dev/null 2>&1; then
		valgrind --error-exitcode=42 --leak-check=full "$OUT" 2>"$T/c07_vg.log" \
			|| { echo "  ERREURS VALGRIND :"; cat "$T/c07_vg.log"; }
	fi
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c07_cc.log"
	exit 1
fi
