#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 4 -- the bonus: the checker program.
#
#  The checker is the only bonus the subject allows ("why not create your own
#  checker?"). It is tested against the official binary on the same inputs, so
#  a divergence is a bug in mine by construction. The subject's own examples are
#  reproduced literally first.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

CH="$ROOT/checker"
OFFICIAL="$ROOT/tests/community/gemartin/checker_linux"
[ -x "$CH" ] || make -C "$ROOT" bonus >/dev/null 2>&1
[ -x "$CH" ] || { echo "checker is missing (make bonus)"; exit 1; }

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

# expect <stdout> <stderr> <args...> with instructions on stdin
expect() {
	local want_out=$1 want_err=$2
	shift 2
	local out err
	out=$(printf '%s' "$INSTRUCTIONS" | timeout 10 "$CH" "$@" 2>"$WORK/e")
	err=$(cat "$WORK/e")
	if [ "$out" != "$want_out" ]; then
		ko "args [$*]: stdout [$out], expected [$want_out]"
		return 1
	fi
	if [ "$err" != "$want_err" ]; then
		ko "args [$*]: stderr [$err], expected [$want_err]"
		return 1
	fi
	ok
}

# same <instructions> <args...> -- my checker and the official one must agree
same() {
	local ops=$1
	shift
	local mine theirs
	mine=$(printf '%s' "$ops" | timeout 10 "$CH" "$@" 2>&1)
	theirs=$(printf '%s' "$ops" | timeout 10 "$OFFICIAL" "$@" 2>&1)
	[ "$mine" = "$theirs" ] && ok || ko "[$*] + [$ops]: mine [$mine], official [$theirs]"
}

echo "== 1. The subject's own examples, verbatim =="
INSTRUCTIONS='rra
pb
sa
rra
pa
'
expect "OK" "" 3 2 1 0
INSTRUCTIONS='sa
rra
pb
'
expect "KO" "" 3 2 1 0
INSTRUCTIONS=''
out=$("$CH" 3 2 one 0 </dev/null 2>&1)
[ "$out" = "Error" ] && ok || ko "checker 3 2 one 0 should print Error, got [$out]"
out=$("$CH" "" 1 </dev/null 2>&1)
[ "$out" = "Error" ] && ok || ko "checker \"\" 1 should print Error, got [$out]"

echo "== 2. No arguments: silence, status 0 =="
out=$("$CH" </dev/null 2>&1)
[ -z "$out" ] && ok || ko "checker without arguments printed [$out]"
"$CH" </dev/null >/dev/null 2>&1
[ $? -eq 0 ] && ok || ko "checker without arguments returned non-zero"

echo "== 3. Sorted / unsorted identification =="
INSTRUCTIONS=''
expect "OK" "" 1 2 3
INSTRUCTIONS=''
expect "KO" "" 3 2 1
INSTRUCTIONS='ra
'
expect "KO" "" 1 2 3
INSTRUCTIONS='pb
pa
'
expect "OK" "" 1 2 3
INSTRUCTIONS='rrr
'
expect "KO" "" 1 2 3

echo "== 4. Instructions that do nothing must not be treated as errors =="
INSTRUCTIONS='sa
'
expect "OK" "" 7
INSTRUCTIONS='pb
pb
pa
pa
'
expect "OK" "" 1
INSTRUCTIONS='ra
rra
'
expect "OK" "" 5

echo "== 5. Every instruction is understood =="
for op in sa sb ss pa pb ra rb rr rra rrb rrr; do
	out=$(printf '%s\n' "$op" | timeout 10 "$CH" 2 1 2>"$WORK/e")
	err=$(cat "$WORK/e")
	if [ -z "$err" ]; then ok; else ko "instruction $op was rejected: [$err]"; fi
done

echo "== 6. Malformed instructions are errors =="
# "sa\nra" is deliberately absent from this list: it is two valid instructions,
# not one malformed one, and the checker is right to accept it.
for bad in "sa " " sa" "SA" "saa" "x" "0" "sa;ra" "ra sa"; do
	out=$(printf "$bad\n" | timeout 10 "$CH" 2 1 2>"$WORK/e")
	err=$(cat "$WORK/e")
	[ "$err" = "Error" ] && ok || ko "malformed [$bad] gave stderr [$err] (expected Error)"
done

echo "== 7. An instruction without its trailing newline is an error =="
out=$(printf 'sa' | timeout 10 "$CH" 2 1 2>"$WORK/e")
err=$(cat "$WORK/e")
[ "$err" = "Error" ] && ok || ko "unterminated instruction gave [$err] (expected Error)"
# ... but a clean empty input is not an error
out=$(printf '' | timeout 10 "$CH" 2 1 2>"$WORK/e")
err=$(cat "$WORK/e")
[ -z "$err" ] && ok || ko "empty input gave stderr [$err]"

echo "== 8. Argument errors are the same as push_swap's =="
for args in "1 1" "a" "2147483648" "1 2 1"; do
	out=$(timeout 10 "$CH" $args </dev/null 2>&1)
	[ "$out" = "Error" ] && ok || ko "checker [$args] gave [$out] (expected Error)"
done
# the quoted empty argument, exactly as the subject writes it
out=$(timeout 10 "$CH" "" 1 </dev/null 2>&1)
[ "$out" = "Error" ] && ok || ko "checker \"\" 1 gave [$out] (expected Error)"
out=$(timeout 10 "$CH" "   " 1 </dev/null 2>&1)
[ "$out" = "Error" ] && ok || ko "checker \"   \" 1 gave [$out] (expected Error)"

echo "== 9. Logical equivalence with the official checker =="
for i in $(seq 1 25); do
	n=$((RANDOM % 10 + 2))
	args=$(seq 1 "$n" | shuf | tr '\n' ' ')
	ops=$(printf 'sa\nsb\nss\npa\npb\nra\nrb\nrr\nrra\nrrb\nrrr\n' | shuf -n 6)
	same "$ops" $args
done

echo "== 10. push_swap feeds the checker on real sizes =="
for n in 2 3 5 10 42 100 200; do
	args=$(seq 1 "$n" | shuf | tr '\n' ' ')
	r=$("$ROOT/push_swap" $args | "$CH" $args 2>&1)
	[ "$r" = "OK" ] && ok || ko "push_swap $n values fed to checker gave [$r]"
done

echo "== 11. A wrong instruction list is caught =="
args=$(seq 1 100 | shuf | tr '\n' ' ')
r=$("$ROOT/push_swap" $args | head -1 | "$CH" $args 2>&1)
[ "$r" = "KO" ] && ok || ko "truncated instruction list gave [$r] (expected KO)"

echo "== 12. Reversed instructions do not sort =="
args=$(seq 1 50 | shuf | tr '\n' ' ')
r=$("$ROOT/push_swap" $args | tac | "$CH" $args 2>&1)
[ "$r" = "KO" ] && ok || ko "reversed instructions gave [$r] (expected KO)"

echo "== 13. The checker does not modify its arguments =="
INSTRUCTIONS=''
out=$(printf '' | "$CH" 3 1 2 2>&1)
[ "$out" = "KO" ] && ok || ko "checker on unsorted gave [$out]"

echo "== 14. Large input does not overflow the line buffer =="
long=$(printf 'ra\n%.0s' $(seq 1 5000))
args=$(seq 1 10 | shuf | tr '\n' ' ')
r=$(printf '%s' "$long" | timeout 20 "$CH" $args 2>&1)
[ -n "$r" ] && ok || ko "5000 instructions produced no verdict"

printf '\n  \033[1;34mbonus\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
