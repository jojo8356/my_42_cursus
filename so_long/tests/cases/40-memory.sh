#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 5 -- memory and robustness.
#
#  Chapter III says the project must not leak, and chapter IV says a malformed
#  map must exit properly -- which includes freeing whatever was already
#  allocated. Both are checked with AddressSanitizer and UBSan, on the real
#  sources: every map of the third-party corpus is fed to both binaries under
#  the sanitisers, plus a scripted winning game and an ESC exit.
#
#  If a sanitiser reports anything, the run is a failure even when the exit
#  status is 0, because "exit properly" is about the memory too.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
STATE=${SOLONG_STATE:-/tmp/solong_ci}

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

command -v cc >/dev/null 2>&1 || { echo "no compiler"; exit 1; }

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$ROOT" || exit 1

echo "== 1. Building both binaries with ASan + UBSan =="
make -C libft >/dev/null 2>&1 || { echo "libft build failed"; exit 1; }
SAN="-fsanitize=address,undefined -fno-omit-frame-pointer -g"
INC="-I include -I libft -I tests/shim"
SOURCES=$(make -s -f Makefile print-sources)
MANDATORY=$(printf '%s\n' "$SOURCES" | sed -n '/^MANDATORY /s///p')
BONUSSRC=$(printf '%s\n' "$SOURCES" | sed -n '/^BONUS /s///p')

build_san() {
	local out=$1
	shift
	local objs=""
	mkdir -p "$WORK/o"
	rm -f "$WORK/o"/*.o
	for f in "$@"; do
		if ! cc -Wall -Wextra $SAN $INC -c "$f" -o "$WORK/o/$(basename "${f%.c}").o" \
			2>"$WORK/build.log"; then
			cat "$WORK/build.log"
			return 1
		fi
	done
	if ! cc -Wall -Wextra $SAN -I tests/shim -c tests/shim/mlx_shim.c \
			-o "$WORK/o/mlx_shim.o" 2>"$WORK/build.log"; then
		cat "$WORK/build.log"
		return 1
	fi
	cc $SAN "$WORK/o"/*.o libft/libft.a -o "$out" || return 1
	return 0
}

if build_san "$WORK/so_long_asan" $MANDATORY; then ok; else ko "sanitised mandatory build"; fi
if build_san "$WORK/so_long_bonus_asan" $BONUSSRC; then ok; else ko "sanitised bonus build"; fi
[ -x "$WORK/so_long_asan" ] || { echo "cannot continue without a binary"; \
	printf '\n  \033[1;34mmemory\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"; exit 1; }

# run_san <binary> <map> [keys] [frames] -- print ONLY sanitizer diagnostics.
#
# The program legitimately writes to stdout: "Error\n<reason>" for a rejected
# map, the move counter for every move, "You lose" for a lost bonus game. None
# of that is a memory problem, so instead of stripping what we expect, keep only
# lines that the sanitisers actually emit. Anything else is the game talking.
run_san() {
	local bin=$1 map=$2 keys=${3:-} frames=${4:-2}
	: > "$WORK/slog"
	MLX_SHIM_LOG="$WORK/slog" MLX_SHIM_FRAMES="$frames" \
		MLX_SHIM_KEYS="$keys" \
		ASAN_OPTIONS=detect_leaks=1:abort_on_error=0 \
		UBSAN_OPTIONS=print_stacktrace=1 \
		timeout 30 "$bin" "$map" 2>&1 \
		| grep -E 'ERROR: (Address|Leak)Sanitizer|SUMMARY:|runtime error:|^=====|^ *#[0-9]+ 0x|^(Direct|Indirect) leak|leaked in' \
		|| true
}

echo "== 2. A full winning game leaks nothing =="
out=$(run_san "$WORK/so_long_asan" "$ROOT/tests/cases/maps/ref.ber" \
	"100 100 100 115 115 100" 9)
if [ -z "$out" ]; then ok; else ko "winning game: $out"; fi

echo "== 3. ESC leaks nothing =="
out=$(run_san "$WORK/so_long_asan" "$ROOT/tests/cases/maps/ref.ber" "65307" 3)
if [ -z "$out" ]; then ok; else ko "ESC exit: $out"; fi

echo "== 4. Every invalid map of the corpus exits cleanly =="
n=0
bad=0
for m in "$ROOT/tests/community/augustobecker/maps/invalid/"*; do
	[ -f "$m" ] || continue
	n=$((n + 1))
	out=$(run_san "$WORK/so_long_asan" "$m")
	if [ -n "$out" ]; then
		bad=$((bad + 1))
		printf '       %s: %s\n' "$(basename "$m")" "$(printf '%s' "$out" | head -2)"
	fi
done
[ "$bad" -eq 0 ] && ok || ko "$bad/$n invalid maps leak or crash"

echo "== 5. Every valid map of the corpus is clean =="
n=0
bad=0
for m in "$ROOT/tests/community/augustobecker/maps/valid/"*.ber; do
	[ -f "$m" ] || continue
	n=$((n + 1))
	out=$(run_san "$WORK/so_long_asan" "$m" "" 1)
	if [ -n "$out" ]; then
		bad=$((bad + 1))
		printf '       %s: %s\n' "$(basename "$m")" "$(printf '%s' "$out" | head -2)"
	fi
done
[ "$bad" -eq 0 ] && ok || ko "$bad/$n valid maps leak or crash"

echo "== 6. Missing and empty files do not leak =="
out=$(run_san "$WORK/so_long_asan" "$WORK/nope.ber")
[ -z "$out" ] && ok || ko "missing file: $out"
: > "$WORK/empty.ber"
out=$(run_san "$WORK/so_long_asan" "$WORK/empty.ber")
[ -z "$out" ] && ok || ko "empty file: $out"

echo "== 7. The bonus does not leak, including a losing game =="
cat > "$WORK/en.ber" <<'EOF'
1111111
1P0C0X1
100EC01
1111111
EOF
out=$(run_san "$WORK/so_long_bonus_asan" "$WORK/en.ber" "" 40)
[ -z "$out" ] && ok || ko "bonus losing game: $out"
out=$(run_san "$WORK/so_long_bonus_asan" "$ROOT/tests/cases/maps/ref.ber" \
	"100 100 100 115 115 100" 9)
[ -z "$out" ] && ok || ko "bonus winning game: $out"

echo "== 8. A map whose file has no trailing newline is accepted =="
printf '11111\n1PCE1\n11111' > "$WORK/nonl.ber"
: > "$WORK/slog"
MLX_SHIM_LOG="$WORK/slog" MLX_SHIM_FRAMES=1 "$STATE/so_long" "$WORK/nonl.ber" \
	>/dev/null 2>&1
[ $? -eq 0 ] && ok || ko "a map without a trailing newline was rejected"

echo "== 9. A huge map does not crash =="
python3 - "$WORK/big.ber" <<'PY'
import sys
w, h = 200, 100
rows = []
for y in range(h):
    if y in (0, h - 1):
        rows.append('1' * w)
        continue
    row = ['1'] + ['0'] * (w - 2) + ['1']
    rows.append(''.join(row))
rows[1] = '1P' + '0' * (w - 3) + '1'
rows[2] = '1E' + '0' * (w - 3) + '1'
rows[3] = '1C' + '0' * (w - 3) + '1'
open(sys.argv[1], 'w').write('\n'.join(rows) + '\n')
PY
out=$(run_san "$WORK/so_long_asan" "$WORK/big.ber" "" 1)
[ -z "$out" ] && ok || ko "200x100 map: $out"

printf '\n  \033[1;34mmemory\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
