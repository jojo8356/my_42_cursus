#!/bin/sh
# Driver C13 : compile les solutions avec la batterie de tests et l'execute
# (valgrind si disponible pour les fuites memoire).
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c13_test_bin"

echo "== C13 =="
if gcc -Wall -Wextra -Wno-strict-prototypes -o "$OUT" "$ROOT/tests/c13_test.c" \
	"$ROOT"/c13/ex*/*.c 2>"$T/c13_cc.log"; then
	if command -v valgrind >/dev/null 2>&1; then
		valgrind --error-exitcode=42 --leak-check=full "$OUT" 2>"$T/c13_vg.log" \
			|| { echo "  ERREURS VALGRIND :"; tail -20 "$T/c13_vg.log"; }
	fi
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c13_cc.log"
	exit 1
fi
