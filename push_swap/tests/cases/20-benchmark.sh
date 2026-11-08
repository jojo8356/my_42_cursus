#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 3 -- the benchmark, from chapter VI.4 of the subject.
#
#  The subject is explicit: full validation (and therefore eligibility for the
#  bonus) requires 100 values under 700 instructions and 500 values under 5500.
#  Anything above those numbers is a failed project, not a slow one.
#
#  Every sample is checked for correctness too. An algorithm that emits 400
#  instructions and does not sort would otherwise look like a triumph here.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

BIN="$ROOT/push_swap"
CHECK="$ROOT/tests/community/gemartin/checker_linux"
BONUS_CHECK="$ROOT/checker"
[ -x "$BIN" ] || make -C "$ROOT" >/dev/null 2>&1

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

SAMPLES=${SAMPLES:-10}

# bench <n> <limit> <label> -- average and worst case over $SAMPLES shuffles
bench() {
	local n=$1 limit=$2 label=$3
	local i ops worst=0 total=0 bad=0
	for i in $(seq 1 "$SAMPLES"); do
		args=$(seq 1 "$n" | shuf | tr '\n' ' ')
		ops=$("$BIN" $args 2>/dev/null | wc -l | tr -d ' ')
		total=$((total + ops))
		[ "$ops" -gt "$worst" ] && worst=$ops
		[ "$ops" -gt "$limit" ] && bad=$((bad + 1))
	done
	if [ "$bad" -eq 0 ]; then
		ok
		printf '  %-28s avg %4d  worst %4d  (limit %d)\n' \
			"$label" $((total / SAMPLES)) "$worst" "$limit"
	else
		ko "$label: $bad/$SAMPLES samples above $limit (worst $worst)"
	fi
}

# sanity <n> -- the instructions must sort, on $SAMPLES shuffles
sanity() {
	local n=$1 i args r bad=0
	for i in $(seq 1 "$SAMPLES"); do
		args=$(seq 1 "$n" | shuf | tr '\n' ' ')
		r=$("$BIN" $args 2>/dev/null | "$CHECK" $args 2>/dev/null)
		[ "$r" = "OK" ] || bad=$((bad + 1))
	done
	if [ "$bad" -eq 0 ]; then ok; else ko "$n values: $bad/$SAMPLES sorts failed the checker"; fi
}

echo "== 1. Full-validation benchmark (chapter VI.4) =="
bench 100 700 "100 random values"
bench 500 5500 "500 random values"

echo "== 2. The same, checked for correctness =="
sanity 100
sanity 500

echo "== 3. Intermediate sizes stay correct and sane =="
for n in 3 5 6 10 25 50 75 150 250 400; do
	args=$(seq 1 "$n" | shuf | tr '\n' ' ')
	r=$("$BIN" $args 2>/dev/null | "$CHECK" $args 2>/dev/null)
	[ "$r" = "OK" ] && ok || ko "$n values: checker said $r"
done

echo "== 4. Adversarial shapes, not just shuffles =="
# already sorted: must be free
ops=$("$BIN" $(seq 1 500) | wc -l | tr -d ' ')
[ "$ops" -eq 0 ] && ok || ko "sorted 500 emitted $ops instructions"

# reversed: the worst case for a naive insertion
ops=$("$BIN" $(seq 500 -1 1) | wc -l | tr -d ' ')
[ "$ops" -lt 5500 ] && ok || ko "reversed 500 took $ops (limit 5500)"

# a rotation of a sorted list: one clean cut
ops=$("$BIN" $(seq 250 500) $(seq 1 249) | wc -l | tr -d ' ')
[ "$ops" -lt 5500 ] && ok || ko "rotated 500 took $ops (limit 5500)"

# Values with a huge spread must behave like their ranks: the sort works on
# ranks, so the two runs should land within a move or two of each other. They
# are not required to be identical, because the permutation is chosen from the
# values themselves and the scaling can tie-break two rotations differently.
# The scaling must preserve the permutation exactly, so the two runs have to be
# byte-identical. Building b with `awk` on a space-separated list is a trap: the
# trailing newline produces an empty field, awk prints a spurious 0, and the
# stack suddenly has a duplicate -- which push_swap rightly refuses. The list is
# filtered to keep this test about scaling and nothing else.
a=$(seq 1 100 | shuf | tr '\n' ' ')
b=$(echo "$a" | tr ' ' '\n' | grep -E '^-?[0-9]+$' | awk '{print $1*1000}' | tr '\n' ' ')
na=$("$BIN" $a 2>/dev/null | wc -l | tr -d ' ')
nb=$("$BIN" $b 2>/dev/null | wc -l | tr -d ' ')
[ "$na" -eq "$nb" ] && ok || ko "scaling changed the plan: $na vs $nb instructions"
[ "$("$BIN" $a 2>/dev/null)" = "$("$BIN" $b 2>/dev/null)" ] && ok \
	|| ko "scaling changed the instruction sequence"

echo "== 5. Stability across repeated runs of the same input =="
args=$(seq 1 100 | shuf | tr '\n' ' ')
first=$("$BIN" $args | wc -l | tr -d ' ')
same=1
for i in 1 2 3; do
	[ "$("$BIN" $args | wc -l | tr -d ' ')" = "$first" ] || same=0
done
[ "$same" -eq 1 ] && ok || ko "same input gave different instruction counts"

echo "== 6. The bonus checker agrees with the official one =="
if [ -x "$BONUS_CHECK" ]; then
	agree=0
	for i in $(seq 1 15); do
		n=$((RANDOM % 12 + 2))
		args=$(seq 1 "$n" | shuf | tr '\n' ' ')
		ops=$(printf 'sa\nsb\nss\npa\npb\nra\nrb\nrr\nrra\nrrb\nrrr\n' | shuf -n 5)
		mine=$(printf '%s\n' "$ops" | "$BONUS_CHECK" $args 2>&1)
		theirs=$(printf '%s\n' "$ops" | "$CHECK" $args 2>&1)
		[ "$mine" = "$theirs" ] || agree=$((agree + 1))
	done
	[ "$agree" -eq 0 ] && ok || ko "$agree/15 disagreements with the official checker"
else
	printf '  (checker not built, run `make bonus` first)\n'
fi

printf '\n  \033[1;34mbenchmark\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
