#!/bin/sh
# Lance la batterie de tests du workspace piscine42.
# Usage : ./run_all_tests.sh [cible...]   (ex: ./run_all_tests.sh c00 rush01)
# Sans argument : lance tout.

ROOT="$(cd "$(dirname "$0")" && pwd)"
FAILED=0
RAN=0

run_driver()
{
	driver="$1"
	if [ ! -f "$driver" ]; then
		return
	fi
	RAN=$((RAN + 1))
	if sh "$driver"; then
		:
	else
		FAILED=$((FAILED + 1))
	fi
}

if [ $# -eq 0 ]; then
	set -- shell00 shell01 c00 c01 c02 c03 c04 c05 c06 c07 c08 c09 \
		c10 c11 c12 c13 rush00 rush01 rush02 bsq exam00 exam01 exam02 exam03
fi

for target in "$@"; do
	run_driver "$ROOT/tests/test_${target}.sh"
done

echo "=========================================="
if [ "$RAN" -eq 0 ]; then
	echo "Aucun driver trouve pour: $*"
	exit 2
fi
if [ "$FAILED" -eq 0 ]; then
	echo "TOUS LES DRIVERS SONT AU VERT ($RAN driver(s))"
	exit 0
fi
echo "$FAILED driver(s) en echec sur $RAN"
exit 1
