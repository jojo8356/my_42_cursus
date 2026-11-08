#!/usr/bin/env bash
# Bonus part: multiple pipes and here_doc, compared with bash.
#
# The reference for N pipes is the shell pipeline itself; the reference for
# here_doc is `cmd << LIMITER | ... >> file`, whose two details matter:
#   - the limiter line is consumed, never transmitted,
#   - the output is *appended* (>>), not truncated (>).

set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

# Run in a scratch directory: the binary is exercised with malformed argument
# lists on purpose and the checkout must not collect their side effects.
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

BIN="$ROOT/pipex"
make -C "$ROOT" bonus >/dev/null 2>&1 || { echo "make bonus failed"; exit 1; }
if ! nm "$BIN" | grep -q px_run_bonus; then
	echo "the binary is not the bonus build"
	exit 1
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$1"; }

printf 'a1\nb2\na3\nc4\nd5\n' > "$TMP/in"
printf 'one 1\ntwo 2\nthree 3\n' > "$TMP/in2"
: > "$TMP/empty"

echo "== 1. Chains of 1 to 10 commands =="
for n in 1 2 3 4 5 6 8 10; do
	args=()
	for ((i = 0; i < n; i++)); do args+=("cat"); done
	$BIN "$TMP/in2" "${args[@]}" "wc -c" "$TMP/o" >/dev/null 2>&1
	expected=$(wc -c < "$TMP/in2")
	if [ "$(cat "$TMP/o")" -eq "$expected" ]; then ok; else ko "$n commands: got '$(cat "$TMP/o")', want $expected"; fi
done

echo "== 2. Byte-exact output on a real chain =="
$BIN "$TMP/in" "grep a" "sort" "tr a-z A-Z" "wc -l" "$TMP/o" >/dev/null 2>&1
eval "< '$TMP/in' grep a | sort | tr a-z A-Z | wc -l > '$TMP/ref'"
cmp -s "$TMP/o" "$TMP/ref" && ok || ko "grep|sort|tr|wc differs"
$BIN "$TMP/in" cat "tail -n 3" "head -n 1" "$TMP/o" >/dev/null 2>&1
eval "< '$TMP/in' cat | tail -n 3 | head -n 1 > '$TMP/ref'"
cmp -s "$TMP/o" "$TMP/ref" && ok || ko "cat|tail|head differs"

echo "== 3. No deadlock with large data through many pipes =="
head -c 300000 /dev/urandom | base64 > "$TMP/big"
for n in 3 7; do
	args=()
	for ((i = 0; i < n; i++)); do args+=("cat"); done
	timeout 20 $BIN "$TMP/big" "${args[@]}" "wc -c" "$TMP/o" >/dev/null 2>&1
	rc=$?
	if [ "$rc" -eq 0 ] && [ "$(cat "$TMP/o")" -eq "$(wc -c < "$TMP/big")" ]; then ok
	else ko "$n cat's on 400 KB: rc=$rc"; fi
done

echo "== 4. Exit status is still the last command's =="
printf '#!/bin/sh\nexit 7\n' > "$TMP/e7"; chmod +x "$TMP/e7"
$BIN "$TMP/in" cat cat "$TMP/e7" "$TMP/o" >/dev/null 2>&1
[ $? -eq 7 ] && ok || ko "last command exits 7, pipex returned $?"
$BIN "$TMP/in" "$TMP/e7" cat cat "$TMP/o" >/dev/null 2>&1
[ $? -eq 0 ] && ok || ko "a failing middle command must not change the status"

echo "== 5. A bad command in the middle of a chain =="
$BIN "$TMP/in" cat "definitely-not-a-command" cat "$TMP/o" >/dev/null 2>&1
[ -f "$TMP/o" ] && ok || ko "outfile not created when a middle command fails"

echo "== 6. here_doc: body, limiter, appending =="
printf 'l1\nl2\nl3\nEOF\nnot transmitted\n' > "$TMP/here.txt"
$BIN here_doc EOF cat "wc -l" "$TMP/h1" < "$TMP/here.txt" >/dev/null 2>&1
[ "$(cat "$TMP/h1")" -eq 3 ] && ok || ko "here_doc body: got '$(cat "$TMP/h1")', want 3"

printf 'l1\nl2\nl3\nEOF\nignored\n' > "$TMP/here.txt"
$BIN here_doc EOF cat cat "$TMP/h2" < "$TMP/here.txt" >/dev/null 2>&1
{ [ "$(cat "$TMP/h2")" = "$(printf 'l1\nl2\nl3')" ] && ok; } \
	|| ko "here_doc must transmit the body only, got '$(cat "$TMP/h2")'"

printf 'PREVIOUS\n' > "$TMP/h3"
$BIN here_doc EOF cat cat "$TMP/h3" < "$TMP/here.txt" >/dev/null 2>&1
head -1 "$TMP/h3" | grep -q PREVIOUS && ok || ko "here_doc must append (>>), not truncate"

echo "== 7. here_doc with a real chain and an empty body =="
printf 'a\nb\nc\nEND\n' > "$TMP/here2"
$BIN here_doc END "grep b" "tr a-z A-Z" "wc -l" "$TMP/h4" < "$TMP/here2" >/dev/null 2>&1
[ "$(cat "$TMP/h4")" -eq 1 ] && ok || ko "here_doc with 3 commands"
printf 'END\n' > "$TMP/here3"
$BIN here_doc END cat "wc -c" "$TMP/h5" < "$TMP/here3" >/dev/null 2>&1
[ "$(cat "$TMP/h5")" -eq 0 ] && ok || ko "here_doc with an empty body"
printf 'a\nENDX\nEND\n' > "$TMP/here4"
$BIN here_doc END cat "wc -l" "$TMP/h6" < "$TMP/here4" >/dev/null 2>&1
[ "$(cat "$TMP/h6")" -eq 2 ] && ok || ko "ENDX must not stop the here_doc of END"

echo "== 8. here_doc from a pipe, and with EOF before the limiter =="
printf 'x\ny\nEND\n' | $BIN here_doc END cat "wc -l" "$TMP/h7" >/dev/null 2>&1
[ "$(cat "$TMP/h7")" -eq 2 ] && ok || ko "here_doc reading from a pipe"
printf 'x\ny\n' | $BIN here_doc END cat "wc -l" "$TMP/h8" >/dev/null 2>&1
[ "$(cat "$TMP/h8")" -eq 2 ] && ok || ko "here_doc with EOF before the limiter must not hang"

echo "== 9. here_doc argument errors =="
$BIN here_doc >/dev/null 2>"$TMP/e"; rc=$?
[ "$rc" -ne 0 ] && grep -qi usage "$TMP/e" && ok || ko "here_doc with no argument (rc=$rc)"
$BIN here_doc EOF >/dev/null 2>"$TMP/e"; rc=$?
[ "$rc" -ne 0 ] && grep -qi usage "$TMP/e" && ok || ko "here_doc with a limiter only (rc=$rc)"

echo "== 10. Multiple pipes exist for real (not N commands through one) =="
# With N-1 pipes, each command really runs in its own process: a command that
# increments a file must be executed exactly once per position.
: > "$TMP/count"
printf '#!/bin/sh\necho x >> %s\ncat\n' "$TMP/count" > "$TMP/tick"; chmod +x "$TMP/tick"
$BIN "$TMP/in" "$TMP/tick" "$TMP/tick" "$TMP/tick" cat "$TMP/o" >/dev/null 2>&1
[ "$(wc -l < "$TMP/count")" -eq 3 ] && ok || ko "3 ticks expected, got $(wc -l < "$TMP/count")"
cmp -s "$TMP/in" "$TMP/o" && ok || ko "data corrupted by the 3 intermediate commands"

printf '\n  \033[1;34mbonus\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
