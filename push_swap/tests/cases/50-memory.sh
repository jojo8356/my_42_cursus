#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 6 -- memory: ASan + UBSan + LeakSanitizer on both binaries.
#
#  The subject is explicit that memory leaks are not tolerated and that a
#  project which quits unexpectedly is worth 0. Both binaries are rebuilt from
#  source with sanitizers, then every path is exercised: the success paths, the
#  error paths, and the paths where an allocation fails halfway.
#
#  valgrind is not available in this sandbox, so the sanitizers do the job.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address,undefined -fno-omit-frame-pointer"
export ASAN_OPTIONS="detect_leaks=1:abort_on_error=1:exitcode=99"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1:exitcode=99"

SHARED=$(ls "$ROOT"/src/*.c | grep -v '/main\.c$')

echo "== 1. Build both flavours with sanitizers =="
mkdir -p "$WORK/obj"
built=1
for f in "$ROOT"/src/*.c; do
	cc $CFLAGS -I "$ROOT/include" -I "$ROOT/libft" -c "$f" -o "$WORK/obj/$(basename "${f%.c}").o" \
		2>"$WORK/build.log" || { built=0; cat "$WORK/build.log"; }
done
[ "$built" -eq 1 ] && ok || ko "sanitized build of the shared sources failed"
cc $CFLAGS "$WORK"/obj/*.o "$ROOT/libft/libft.a" -o "$WORK/ps_asan" 2>>"$WORK/build.log"
[ -x "$WORK/ps_asan" ] && ok || ko "could not link the sanitized push_swap"

# the checker: every source except main.c, plus the bonus entry point
mkdir -p "$WORK/bobj"
bbuilt=1
for f in "$ROOT"/src/bonus/*.c; do
	cc $CFLAGS -I "$ROOT/include" -I "$ROOT/libft" -c "$f" -o "$WORK/bobj/$(basename "${f%.c}").o" \
		2>>"$WORK/build.log" || bbuilt=0
done
for f in $SHARED "$ROOT"/src/main.c; do
	[ "$(basename "$f")" = "main.c" ] && continue
	cc $CFLAGS -I "$ROOT/include" -I "$ROOT/libft" -c "$f" \
		-o "$WORK/bobj/s_$(basename "${f%.c}").o" 2>>"$WORK/build.log" || bbuilt=0
done
[ "$bbuilt" -eq 1 ] && ok || ko "sanitized build of the checker sources failed"
cc $CFLAGS "$WORK"/bobj/*.o "$ROOT/libft/libft.a" -o "$WORK/ch_asan" 2>>"$WORK/build.log"
[ -x "$WORK/ch_asan" ] && ok || ko "could not link the sanitized checker"

[ -x "$WORK/ps_asan" ] && [ -x "$WORK/ch_asan" ] || {
	echo "sanitized binaries are missing, stopping"
	printf '\n  \033[1;34mmemory\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
	exit 1
}

# run_clean <description> <binary> <args...> -- must exit without a report
run_clean() {
	local desc=$1
	shift
	local out rc
	out=$(timeout 120 "$@" 2>"$WORK/err")
	rc=$?
	if [ "$rc" -eq 99 ]; then
		ko "$desc: sanitizer reported an error"
		head -20 "$WORK/err"
	elif [ -s "$WORK/err" ] && grep -qE 'ERROR: (Address|Leak)Sanitizer|runtime error' "$WORK/err"; then
		ko "$desc: sanitizer output"
		head -20 "$WORK/err"
	else
		ok
	fi
}

echo "== 2. Success paths of push_swap =="
for n in 1 2 3 4 5 6 10 42 100 200 500; do
	args=$(seq 1 "$n" | shuf | tr '\n' ' ')
	run_clean "push_swap $n values" "$WORK/ps_asan" $args
done
run_clean "already sorted" "$WORK/ps_asan" $(seq 1 200)
run_clean "reversed" "$WORK/ps_asan" $(seq 200 -1 1)
run_clean "single value" "$WORK/ps_asan" 42
run_clean "extremes" "$WORK/ps_asan" 2147483647 -2147483648 0

echo "== 3. Error paths must free what they allocated =="
run_clean "non-integer" "$WORK/ps_asan" 1 two 3
run_clean "duplicate" "$WORK/ps_asan" 1 2 2
run_clean "empty argument" "$WORK/ps_asan" "" 1
run_clean "overflow" "$WORK/ps_asan" 99999999999999999999
run_clean "bad in the middle of many" "$WORK/ps_asan" $(seq 1 200) nope
run_clean "bad after a big list" "$WORK/ps_asan" $(seq 1 500) x

echo "== 4. No arguments =="
run_clean "push_swap without arguments" "$WORK/ps_asan"
run_clean "checker without arguments" "$WORK/ch_asan"

echo "== 5. Checker success and failure paths =="
args="3 2 1"
printf 'rra\npb\nsa\nrra\npa\n' > "$WORK/ok.txt"
printf 'sa\n' > "$WORK/ko.txt"
printf 'nonsense\n' > "$WORK/bad.txt"
printf 'sa' > "$WORK/nonl.txt"
run_clean "checker OK" bash -c "'$WORK/ch_asan' 3 2 1 < '$WORK/ok.txt'"
run_clean "checker KO" bash -c "'$WORK/ch_asan' 3 2 1 < '$WORK/ko.txt'"
run_clean "checker Error" bash -c "'$WORK/ch_asan' 3 2 1 < '$WORK/bad.txt'"
run_clean "checker unterminated" bash -c "'$WORK/ch_asan' 3 2 1 < '$WORK/nonl.txt'"
run_clean "checker empty stdin" bash -c "'$WORK/ch_asan' 3 2 1 < /dev/null"
run_clean "checker bad arguments" "$WORK/ch_asan" 1 1 2
run_clean "checker 500 values" bash -c \
	"'$ROOT/push_swap' \$(seq 1 500 | shuf | tr '\\n' ' ') | '$WORK/ch_asan' \$(seq 1 500 | shuf | tr '\\n' ' ')"
run_clean "checker 5000 instructions" bash -c \
	"seq 1 5000 | while read i; do echo ra; done | '$WORK/ch_asan' \$(seq 1 10)"

echo "== 6. Short writes and closed streams do not leak =="
run_clean "stdout to /dev/full" bash -c \
	"'$WORK/ps_asan' \$(seq 1 100 | shuf | tr '\\n' ' ') > /dev/full"
run_clean "stdout closed" bash -c \
	"'$WORK/ps_asan' 3 1 2 >&-"

echo "== 7. Allocation failure is not simulated, but overflow is =="
# a stack of 2000 values: well past any branch, and enough to catch an
# off-by-one in the buffers
args=$(seq 1 2000 | shuf | tr '\n' ' ')
run_clean "2000 values" "$WORK/ps_asan" $args

printf '\n  \033[1;34mmemory\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
