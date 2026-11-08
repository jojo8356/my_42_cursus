#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 5 -- robustness: what a hostile evaluation would try.
#
#  Argument shapes that a shell produces by accident, inputs that are not
#  numbers, sizes at the boundaries of the algorithm's branches, and the cases
#  where a crash would be worse than a wrong answer. Nothing here is exotic;
#  every case is something a reviewer reaches for in the first five minutes.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

BIN="$ROOT/push_swap"
CH="$ROOT/checker"
CHECK="$ROOT/tests/community/gemartin/checker_linux"
[ -x "$BIN" ] || make -C "$ROOT" >/dev/null 2>&1

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

# no_crash <description> <command...> -- must not segfault or hang
no_crash() {
	local desc=$1
	shift
	timeout 20 "$@" >/dev/null 2>&1
	local rc=$?
	if [ "$rc" -eq 139 ]; then
		ko "$desc: segmentation fault"
	elif [ "$rc" -eq 124 ]; then
		ko "$desc: timed out (hang)"
	elif [ "$rc" -gt 128 ]; then
		ko "$desc: killed by signal $((rc - 128))"
	else
		ok
	fi
}

# sorted_by <args...> -- the output must sort, whatever it contains
sorted_by() {
	local r
	r=$("$BIN" "$@" 2>/dev/null | "$CHECK" "$@" 2>/dev/null)
	[ "$r" = "OK" ] && ok || ko "not sorted: $* (checker said $r)"
}

echo "== 1. Argument shapes a shell produces by accident =="
no_crash "quoted whole list" "$BIN" "5 4 3 2 1"
no_crash "many spaces" "$BIN" "5    4    3"
no_crash "tabs inside" "$BIN" "$(printf '5\t4\t3')"
no_crash "newline inside" "$BIN" "$(printf '5\n4\n3')"
no_crash "leading plus" "$BIN" +5 +4 +3
no_crash "leading zeros" "$BIN" 0005 0004 0003
sorted_by "5 4 3 2 1"
sorted_by +5 +4 +3
sorted_by 0005 0004 0003

r=$("$BIN" "5    4    3" 2>/dev/null | "$CHECK" 5 4 3 2>/dev/null)
[ "$r" = "OK" ] && ok || ko "multiple spaces inside one argument broke parsing ($r)"

echo "== 2. Shapes that must be refused, without crashing =="
refuse() {
	local desc=$1
	shift
	local out err
	out=$(timeout 10 "$BIN" "$@" 2>"$WORK/e")
	err=$(cat "$WORK/e")
	[ "$out" = "" ] && [ "$err" = "Error" ] && ok || ko "$desc: stdout [$out] stderr [$err]"
}
refuse "empty" ""
refuse "spaces" "   "
refuse "tab" "$(printf '\t')"
refuse "plus" "+"
refuse "minus" "-"
refuse "double sign" "--5"
refuse "double plus" "++5"
refuse "sign in middle" "5-"
refuse "letter" "5a"
refuse "comma" "5,4"
refuse "float" "5.0"
refuse "exp" "5e3"
refuse "hex" "0x1f"
refuse "octal looking" "0755x"

echo "== 3. Integer boundaries =="
no_crash "INT_MAX alone" "$BIN" 2147483647
no_crash "INT_MIN alone" "$BIN" -2147483648
sorted_by 2147483647 -2147483648
refuse "INT_MAX + 1" 2147483648
refuse "INT_MIN - 1" -2147483649
refuse "way over" 999999999999999999999
refuse "way under" -999999999999999999999
r=$("$BIN" -2147483648 -1 0 1 2147483647 | "$CHECK" -2147483648 -1 0 1 2147483647 2>/dev/null)
[ "$r" = "OK" ] && ok || ko "extremes not sorted ($r)"

echo "== 4. Size boundaries of the algorithm branches =="
for n in 1 2 3 4 5 6 7 8 9 10 50 100 101 199 200 201 499 500 501; do
	args=$(seq 1 "$n" | shuf | tr '\n' ' ')
	sorted_by $args
done

echo "== 5. Inputs that stress the cost model =="
# every value already in place except the last
args=$(seq 2 100 | tr '\n' ' ')
sorted_by $args 1
# one swap away from sorted
args="2 1 $(seq 3 100 | tr '\n' ' ')"
sorted_by $args
# two interleaved sorted runs
args="$(seq 1 2 200 | tr '\n' ' ') $(seq 2 2 200 | tr '\n' ' ')"
sorted_by $args
# all values negative
args=$(seq -100 -1 | shuf | tr '\n' ' ')
sorted_by $args
# a tight cluster
args=$(seq 1000000 1000100 | shuf | tr '\n' ' ')
sorted_by $args

echo "== 6. Very large single values do not overflow the cost arithmetic =="
args="2147483647 -2147483648 0 1 -1"
sorted_by $args

echo "== 7. Repeated runs are stable and leak-free =="
args=$(seq 1 100 | shuf | tr '\n' ' ')
first=$("$BIN" $args | wc -l)
same=1
for i in 1 2 3 4 5; do
	[ "$("$BIN" $args | wc -l)" = "$first" ] || same=0
done
[ "$same" -eq 1 ] && ok || ko "repeated runs of the same input differ"

echo "== 8. Concurrent runs do not interfere =="
pids=""
for i in 1 2 3 4 5; do
	( "$BIN" $(seq 1 100 | shuf | tr '\n' ' ') > "$WORK/out$i" 2>&1 ) &
	pids="$pids $!"
done
crashed=0
for p in $pids; do
	wait "$p" || crashed=$((crashed + 1))
done
[ "$crashed" -eq 0 ] && ok || ko "$crashed/5 concurrent runs failed"

echo "== 9. Output is not truncated on a big run =="
args=$(seq 1 500 | shuf | tr '\n' ' ')
lines=$("$BIN" $args | wc -l)
r=$("$BIN" $args | "$CHECK" $args 2>/dev/null)
[ "$r" = "OK" ] && [ "$lines" -gt 100 ] && ok || ko "500-value run: $lines lines, checker $r"

echo "== 10. Closing stdout makes the binary exit quietly =="
"$BIN" $(seq 1 100 | shuf | tr '\n' ' ') > /dev/full 2>"$WORK/e"
rc=$?
[ "$rc" -ne 139 ] && ok || ko "segfault writing to a full device"

echo "== 11. The checker handles hostile instruction streams =="
if [ -x "$CH" ]; then
	for stream in "" "\n" "sa\n\nra" "sa\r" "$(printf 'ra\n%.0s' $(seq 1 2000))"; do
		out=$(printf '%s\n' "$stream" | timeout 10 "$CH" 3 1 2 2>&1)
		case "$out" in
			OK|KO|Error) ok ;;
			*) ko "checker gave [$out] for a hostile stream" ;;
		esac
	done
	# a very long line must not overflow the fixed buffer
	long=$(head -c 5000 /dev/zero | tr '\0' 'a')
	out=$(printf '%s\n' "$long" | timeout 10 "$CH" 3 1 2 2>&1)
	[ "$out" = "Error" ] && ok || ko "5000-char instruction gave [$out] (expected Error)"
else
	printf '  (checker not built)\n'
fi

echo "== 12. Nothing is written outside stdout and stderr =="
before=$(ls)
"$BIN" $(seq 1 50 | shuf | tr '\n' ' ') >/dev/null 2>&1
after=$(ls)
[ "$before" = "$after" ] && ok || ko "push_swap created files: $(comm -13 <(echo "$before") <(echo "$after") | tr '\n' ' ')"

printf '\n  \033[1;34mrobustness\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
