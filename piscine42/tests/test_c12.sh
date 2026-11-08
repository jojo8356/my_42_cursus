#!/bin/sh
# Driver C12 : compile les solutions avec la batterie de tests et l'execute
# (valgrind si disponible pour les fuites memoire).
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
T="${TMPDIR:-/tmp}"
OUT="$T/c12_test_bin"

echo "== C12 =="
if gcc -Wall -Wextra -Wno-strict-prototypes -o "$OUT" "$ROOT/tests/c12_test.c" \
	"$ROOT"/c12/ex*/*.c 2>"$T/c12_cc.log"; then
	if command -v valgrind >/dev/null 2>&1; then
		valgrind --error-exitcode=42 --leak-check=full "$OUT" 2>"$T/c12_vg.log" \
			|| { echo "  ERREURS VALGRIND :"; tail -20 "$T/c12_vg.log"; }
	fi
	"$OUT"
	RC=$?
	rm -f "$OUT"
	exit $RC
else
	echo "  ERREUR DE COMPILATION :"
	cat "$T/c12_cc.log"
	exit 1
fi
