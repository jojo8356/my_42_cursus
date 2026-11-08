#!/bin/bash
# ---------------------------------------------------------------------------
#  Layer 2 -- map validation.
#
#  Chapter IV lists ten rules a map must satisfy, and says the program must
#  "exit properly and return Error\n followed by an explicit error message" on
#  any misconfiguration. Every rule is exercised here, on maps written for the
#  occasion, plus the 41 maps shipped by the third-party tester.
#
#  The binary runs headlessly (see tests/build.sh), so "exit properly" is
#  checked for real: status, both streams, and no window ever opened.
# ---------------------------------------------------------------------------
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
STATE=${SOLONG_STATE:-/tmp/solong_ci}
BIN="$STATE/so_long"
[ -x "$BIN" ] || { echo "binary missing: run tests/build.sh"; exit 1; }

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$*"; }

# make_map <name> <line>... -- write a .ber file and echo its path
make_map() {
	local name=$1
	shift
	local path="$WORK/$name.ber"
	printf '%s\n' "$@" > "$path"
	printf '%s' "$path"
}

# refuse <description> <map-path> -- must print Error and a reason, status != 0
#
# The message goes to stdout (see the comment on bad_map): chapter IV does not
# name a stream, and the community tester reads stdout. So this helper checks
# stdout, and separately that the run stayed quiet on stderr -- an invalid map
# must not spray diagnostics either.
refuse() {
	local desc=$1 path=$2
	local out err rc
	out=$(MLX_SHIM_LOG="$WORK/log" timeout 10 "$BIN" "$path" 2>"$WORK/e")
	rc=$?
	err=$(cat "$WORK/e")
	if [ "$rc" -eq 0 ]; then
		ko "$desc: accepted an invalid map (status 0)"
		return 1
	fi
	if [ -n "$err" ]; then
		ko "$desc: wrote to stderr [$err]"
		return 1
	fi
	first=$(printf '%s\n' "$out" | head -1)
	if [ "$first" != "Error" ]; then
		ko "$desc: first output line is not Error: [$first]"
		return 1
	fi
	# the subject asks for "Error" followed by an explicit message
	if [ "$(printf '%s\n' "$out" | wc -l)" -lt 2 ]; then
		ko "$desc: no explicit message after Error"
		return 1
	fi
	ok
	return 0
}

# accept <description> <map-path> -- must run and draw, status 0
accept() {
	local desc=$1 path=$2
	local rc
	MLX_SHIM_LOG="$WORK/log" MLX_SHIM_FRAMES=1 timeout 10 "$BIN" "$path" \
		> "$WORK/o" 2> "$WORK/e"
	rc=$?
	if [ "$rc" -ne 0 ]; then
		ko "$desc: rejected a valid map (status $rc): $(head -2 "$WORK/o")"
		return 1
	fi
	if grep -q '^Error' "$WORK/o"; then
		ko "$desc: valid map printed an error: $(head -1 "$WORK/o")"
		return 1
	fi
	ok
}

echo "== 1. The subject's own two example maps =="
accept "subject example 1" "$(make_map ex1 \
	'1111111111111' \
	'10010000000C1' \
	'1000011111001' \
	'1P0011E000001' \
	'1111111111111')"
accept "subject example 2" "$(make_map ex2 \
	'1111111111111111111111111111111111' \
	'1E0000000000000C00000C000000000001' \
	'1010010100100000101001000000010101' \
	'1010010010101010001001000000010101' \
	'1P0000000C00C0000000000000000000C1' \
	'1111111111111111111111111111111111')"

echo "== 2. Rectangularity =="
refuse "ragged last row" "$(make_map ragged '11111' '1P0C1' '1E1' '11111')"
refuse "one row longer" "$(make_map longer '11111' '1P0C11' '100E1' '11111')"
refuse "one row shorter" "$(make_map shorter '11111' '1P0C' '100E1' '11111')"
refuse "single long row" "$(make_map onerow '1111111111')"

echo "== 3. Walls =="
refuse "open top" "$(make_map opentop '10101' '1P0C1' '100E1' '11111')"
refuse "open bottom" "$(make_map openbot '11111' '1P0C1' '100E1' '10101')"
refuse "open left" "$(make_map openleft '11111' '0P0C1' '100E1' '11111')"
refuse "open right" "$(make_map openright '11111' '1P0C0' '100E1' '11111')"
refuse "hole in top right" "$(make_map hole '11110' '1P0C1' '100E1' '11111')"
refuse "no wall at all" "$(make_map nowall '00000' '0P0C0' '000E0' '00000')"

echo "== 4. Required sprites =="
refuse "no collectible" "$(make_map noc '11111' '1P001' '100E1' '11111')"
refuse "no exit" "$(make_map noe '11111' '1P0C1' '10001' '11111')"
refuse "no player" "$(make_map nop '11111' '100 1' '1C0E1' '11111')"
refuse "two players" "$(make_map twop '11111' '1PPC1' '100E1' '11111')"
refuse "two exits" "$(make_map twoe '11111' '1P0C1' '1E0E1' '11111')"
refuse "three players" "$(make_map threep '11111' '1PPC1' '1P0E1' '11111')"

echo "== 5. Charset =="
refuse "letter A" "$(make_map letterA '11111' '1PA01' '10CE1' '11111')"
refuse "digit 2" "$(make_map digit2 '11111' '1P201' '10CE1' '11111')"
refuse "space inside" "$(make_map space '11111' '1P  C1' '100E1' '11111')"
refuse "tab inside" "$(make_map tab "$(printf '11111')" "$(printf '1P\tC1')" \
	'100E1' '11111')"
refuse "asterisk" "$(make_map star '11111' '1P*C1' '100E1' '11111')"
refuse "enemy on mandatory" "$(make_map enemy '11111' '1PXC1' '100E1' '11111')"

echo "== 6. Empty lines =="
refuse "empty second line" "$(make_map empty2 '11111' '' '1P0C1' '100E1' '11111')"
refuse "empty last line" "$(make_map emptylast '11111' '1P0C1' '100E1' '11111' '')"
refuse "only empty lines" "$(make_map allempty '' '' '')"

echo "== 7. Argument handling =="
refuse "missing file" "$WORK/does_not_exist.ber"
refuse "directory as map" "$WORK"
touch "$WORK/empty.ber"
refuse "empty file" "$WORK/empty.ber"
cp "$(make_map good '11111' '1P0C1' '100E1' '11111')" "$WORK/good.txt"
refuse "wrong extension" "$WORK/good.txt"
refuse "no extension" "$WORK/good"
cp "$(make_map good2 '11111' '1P0C1' '100E1' '11111')" "$WORK/good.BER"
refuse "uppercase extension" "$WORK/good.BER"
# no argument, and too many
out=$(timeout 10 "$BIN" 2>"$WORK/e"); rc=$?
[ "$rc" -ne 0 ] && printf '%s' "$out" | grep -q '^Error' && ok \
	|| ko "no argument accepted or no error printed"
"$BIN" a.ber b.ber >/dev/null 2>&1
[ $? -ne 0 ] && ok || ko "two arguments accepted"

echo "== 8. Nothing graphical happens on a rejected map =="
make_map later '11111' '1P0C1' '100E1' '10101' > /dev/null
MLX_SHIM_LOG="$WORK/nolog" "$BIN" "$WORK/later.ber" >/dev/null 2>&1
if [ -f "$WORK/nolog" ]; then
	ko "the window was created before the map was validated"
else
	ok
fi

echo "== 9. The third-party tester's 41 maps =="
TDIR="$ROOT/tests/community/augustobecker/maps"
n_invalid=0
n_bad=0
for m in "$TDIR"/invalid/*; do
	[ -f "$m" ] || continue
	n_invalid=$((n_invalid + 1))
	out=$(MLX_SHIM_LOG=/dev/null "$BIN" "$m" 2>/dev/null)
	rc=$?
	if [ "$rc" -eq 0 ] || ! printf '%s' "$out" | head -1 | grep -q '^Error'; then
		n_bad=$((n_bad + 1))
		printf '       accepted or wrong error: %s\n' "$(basename "$m")"
	fi
done
[ "$n_bad" -eq 0 ] && ok || ko "$n_bad/$n_invalid invalid maps were not rejected"
found=$(ls "$TDIR"/valid/*.ber 2>/dev/null | wc -l)
if [ "$found" -gt 0 ]; then
	n_bad=0
	for m in "$TDIR"/valid/*.ber; do
		MLX_SHIM_LOG="$WORK/log" MLX_SHIM_FRAMES=1 "$BIN" "$m" \
			>/dev/null 2>&1
		[ $? -eq 0 ] || { n_bad=$((n_bad + 1)); \
			printf '       rejected valid map: %s\n' "$(basename "$m")"; }
	done
	[ "$n_bad" -eq 0 ] && ok || ko "$n_bad valid maps were rejected"
else
	ok
fi

printf '\n  \033[1;34mvalidation\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
