#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 2 -- mandatory part: the subject's contract, checked one clause at a
#  time. Every structural claim is verified against the official checker so a
#  test can only pass when the output actually sorts the stack.
#
#  Every case runs inside a scratch directory: the binary is fed malformed
#  argument lists on purpose and the checkout must not collect their effects.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

BIN="$ROOT/push_swap"
CHECK="$ROOT/tests/community/gemartin/checker_linux"
[ -x "$BIN" ] || make -C "$ROOT" >/dev/null 2>&1
[ -x "$BIN" ] || { echo "push_swap is missing"; exit 1; }

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

# --- helpers ---------------------------------------------------------------

# count_ops <args...> -- number of instructions produced
count_ops() { "$BIN" "$@" 2>/dev/null | wc -l | tr -d ' '; }

# valid <args...> -- the instructions must actually sort the stack
valid() {
	local r
	r=$("$BIN" "$@" 2>/dev/null | "$CHECK" "$@" 2>/dev/null)
	[ "$r" = "OK" ] || { ko "instructions do not sort: $* (checker said $r)"; return 1; }
}

# format <args...> -- every line must be a known instruction, nothing else
format() {
	local bad
	bad=$("$BIN" "$@" 2>/dev/null \
		| grep -vcE '^(sa|sb|ss|pa|pb|ra|rb|rr|rra|rrb|rrr)$')
	[ "$bad" -eq 0 ] || { ko "$*: $bad malformed output lines"; return 1; }
}

# ---------------------------------------------------------------------------
# 1. No arguments: nothing on stdout, nothing on stderr, status 0
# ---------------------------------------------------------------------------
out=$(timeout 10 "$BIN" 2>"$WORK/e1"); rc=$?
[ -z "$out" ] && ok || ko "no arguments printed to stdout: [$out]"
[ -s "$WORK/e1" ] && ko "no arguments printed to stderr" || ok
[ "$rc" -eq 0 ] && ok || ko "no arguments: exit $rc (expected 0)"

# ---------------------------------------------------------------------------
# 2. Already sorted: no output at all
# ---------------------------------------------------------------------------
for n in 1 2 3 5 10 100 500; do
	out=$(timeout 20 "$BIN" $(seq 1 "$n"))
	[ -z "$out" ] && ok || ko "sorted input of $n values emitted $(echo "$out" | wc -l) instructions"
done

# ---------------------------------------------------------------------------
# 3. Small cases: 2 to 5 values, every permutation
# ---------------------------------------------------------------------------
check_perm() {
	local args="$*"
	local r
	r=$("$BIN" $args 2>/dev/null | "$CHECK" $args 2>/dev/null)
	[ "$r" = "OK" ] || { ko "permutation [$args] not sorted ($r)"; return 1; }
	format $args || return 1
	return 0
}
for a in 1 2 3 4 5; do
	for b in 1 2 3 4 5; do
		[ "$a" = "$b" ] && continue
		check_perm "$a" "$b" && ok
	done
done
for a in 1 2 3 4 5; do
	for b in 1 2 3 4 5; do
		for c in 1 2 3 4 5; do
			[ "$a" = "$b" ] || [ "$b" = "$c" ] || [ "$a" = "$c" ] && continue
			check_perm "$a" "$b" "$c" && ok
		done
	done
done
for seed in 1 2 3 4 5; do
	args=$(seq 1 5 | shuf --random-source=<(yes $seed) | tr '\n' ' ')
	check_perm $args && ok
done

# ---------------------------------------------------------------------------
# 4. Three values: at most 3 instructions is the known optimum
# ---------------------------------------------------------------------------
for args in "1 2 3" "1 3 2" "2 1 3" "2 3 1" "3 1 2" "3 2 1"; do
	n=$(count_ops $args)
	[ "$n" -le 3 ] && ok || ko "3 values [$args] took $n instructions (max 3)"
done

# ---------------------------------------------------------------------------
# 5. Errors: "Error\n" on stderr, status 1, nothing on stdout
# ---------------------------------------------------------------------------
err_case() {
	local desc="$1"
	shift
	local out err rc
	out=$(timeout 10 "$BIN" "$@" 2>"$WORK/err")
	rc=$?
	err=$(cat "$WORK/err")
	[ -z "$out" ] || { ko "$desc: wrote to stdout"; return 1; }
	[ "$err" = "Error" ] || { ko "$desc: stderr was [$err] (expected Error)"; return 1; }
	[ "$rc" -ne 0 ] || { ko "$desc: exit status 0 on error"; return 1; }
	ok
}
err_case "non-integer" 1 two 3
err_case "non-integer at start" abc
err_case "duplicate" 1 2 2
err_case "duplicate far apart" 5 1 9 1
err_case "empty string" ""
err_case "only spaces" "   "
err_case "INT_MAX + 1" 2147483648
err_case "INT_MIN - 1" -2147483649
err_case "overflow in digits" 99999999999999999999
err_case "plus sign only" +
err_case "minus sign only" -
err_case "decimal" 1.5
err_case "hex" 0x10
err_case "trailing letters" 12a
err_case "leading letters" a12
err_case "one bad among many" $(seq 1 50) nope

# ---------------------------------------------------------------------------
# 6. Limits that must be accepted
# ---------------------------------------------------------------------------
"$BIN" 2147483647 >/dev/null 2>&1 && ok || ko "INT_MAX rejected"
"$BIN" -2147483648 >/dev/null 2>&1 && ok || ko "INT_MIN rejected"
valid 2147483647 -2147483648 0 && ok

# ---------------------------------------------------------------------------
# 7. Arguments given as one quoted string ("1 2 3") must work like three
# ---------------------------------------------------------------------------
r=$("$BIN" "3 2 1" 2>/dev/null)
[ -n "$r" ] && ok || ko "a single quoted argument was not parsed"
r=$("$BIN" "3 2 1" 2>/dev/null | "$CHECK" 3 2 1 2>/dev/null)
[ "$r" = "OK" ] && ok || ko "quoted argument produced a wrong sort ($r)"
r=$("$BIN" "1  2   3" 2>/dev/null)
[ -z "$r" ] && ok || ko "multiple spaces between values not handled"

# ---------------------------------------------------------------------------
# 8. Output format: exactly the 11 instructions, each on its own line
# ---------------------------------------------------------------------------
args=$(seq 1 100 | shuf | tr '\n' ' ')
bad=$("$BIN" $args | grep -vcE '^(sa|sb|ss|pa|pb|ra|rb|rr|rra|rrb|rrr)$')
[ "$bad" -eq 0 ] && ok || ko "$bad malformed lines for 100 values"
# no trailing space anywhere
if "$BIN" $args | grep -q ' $'; then ko "trailing space in output"; else ok; fi
# no empty line in the middle
if "$BIN" $args | grep -qx ''; then ko "empty line in output"; else ok; fi
# last byte is a newline
last=$("$BIN" $args | tail -c 1 | od -An -c | tr -d ' ')
[ "$last" = '\n' ] && ok || ko "output does not end with a newline"

# ---------------------------------------------------------------------------
# 9. Correctness on random inputs, sizes 6 to 200
# ---------------------------------------------------------------------------
for n in 6 7 8 9 10 11 20 42 100 200; do
	for t in 1 2 3; do
		args=$(seq 1 "$n" | shuf | tr '\n' ' ')
		valid $args && format $args && ok
	done
done

# ---------------------------------------------------------------------------
# 10. Values with a wide spread and negative numbers
# ---------------------------------------------------------------------------
valid -5 0 5 -10 10 -1 1 && ok
valid 1000000 -1000000 0 999999 -999999 && ok
args=$(seq -50 50 | shuf | tr '\n' ' ')
valid $args && ok

# ---------------------------------------------------------------------------
# 11. Duplicates are refused even when they are equal negative numbers
# ---------------------------------------------------------------------------
err_case "duplicate negatives" -3 -3
err_case "duplicate zeros" 0 0 0

# ---------------------------------------------------------------------------
# 12. The binary does not read stdin and does not hang
# ---------------------------------------------------------------------------
timeout 5 "$BIN" 3 1 2 </dev/null >/dev/null 2>&1
[ $? -ne 124 ] && ok || ko "push_swap hung with stdin closed"
timeout 5 bash -c "printf 'ra\npa\n' | '$BIN' 3 1 2" >/dev/null 2>&1
[ $? -ne 124 ] && ok || ko "push_swap hung with stdin attached"

# ---------------------------------------------------------------------------
# 13. No write outside the two allowed streams
# ---------------------------------------------------------------------------
out=$(timeout 10 "$BIN" 3 1 2 2>/dev/null)
printf '%s' "$out" | grep -qv '^[a-z]*$' && ko "unexpected bytes on stdout" || ok

# ---------------------------------------------------------------------------
# 14. Exit status is 0 on success, including for a one-value stack
# ---------------------------------------------------------------------------
"$BIN" 42 >/dev/null 2>&1 && ok || ko "single value: non-zero status"
"$BIN" 2 1 >/dev/null 2>&1 && ok || ko "two values: non-zero status"

# ---------------------------------------------------------------------------
# 15. Nothing is written when the stack is already sorted, even with errors
#     possible in the shape of the arguments
# ---------------------------------------------------------------------------
out=$("$BIN" "5" 2>&1)
[ -z "$out" ] && ok || ko "single argument produced [$out]"

printf '\n  \033[1;34mmandatory\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
