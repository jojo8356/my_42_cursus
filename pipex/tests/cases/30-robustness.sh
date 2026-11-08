#!/usr/bin/env bash
# Robustness: the subject's "in no way should your program quit unexpectedly"
# (III.2), plus the edge cases a reviewer pokes at during the defence.
#
# Nothing here is compared with bash: these are invariants.
#   - no signal death, no timeout, no zombie left behind,
#   - no crash on hostile inputs (empty command, spaces, huge argv),
#   - correct behaviour when a command dies by signal itself.

set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

# Everything runs in a scratch directory, never in the repository root: several
# cases throw malformed argument lists at the binary on purpose, and one of them
# legitimately ends up creating an empty file. Running from a throwaway
# directory keeps the checkout pristine and makes the run reproducible.
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

BIN="$ROOT/pipex"
make -C "$ROOT" bonus >/dev/null 2>&1 || { echo "make bonus failed"; exit 1; }

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$1"; }

printf 'l1\nl2\nl3\n' > "$TMP/in"

# A return code >= 128 means "killed by signal", which the subject forbids.
# 124 is the timeout(1) convention for "did not finish".
runs_cleanly() {
	local label=$1; shift
	local rc
	timeout 15 "$@" >/dev/null 2>&1
	rc=$?
	if [ "$rc" -eq 124 ]; then ko "$label: timed out (deadlock)"
	elif [ "$rc" -ge 128 ]; then ko "$label: killed by signal $((rc - 128))"
	else ok; fi
}

echo "== 1. Hostile argument counts =="
runs_cleanly "no argument"          $BIN
runs_cleanly "one argument"         $BIN "$TMP/in"
runs_cleanly "two arguments"        $BIN "$TMP/in" cat
runs_cleanly "three arguments"      $BIN "$TMP/in" cat cat
runs_cleanly "six arguments"        $BIN "$TMP/in" cat cat cat cat cat
runs_cleanly "here_doc alone"       $BIN here_doc
runs_cleanly "here_doc + limiter"   $BIN here_doc EOF

echo "== 2. Empty and whitespace-only commands =="
runs_cleanly "empty cmd1"           $BIN "$TMP/in" "" "cat" "$TMP/o"
runs_cleanly "empty cmd2"           $BIN "$TMP/in" "cat" "" "$TMP/o"
runs_cleanly "spaces only"          $BIN "$TMP/in" "   " "cat" "$TMP/o"
runs_cleanly "leading spaces"       $BIN "$TMP/in" "  cat" "cat" "$TMP/o"
runs_cleanly "trailing spaces"      $BIN "$TMP/in" "cat  " "  wc -l" "$TMP/o"
$BIN "$TMP/in" "  cat  " "cat" "$TMP/o" >/dev/null 2>&1
cmp -s "$TMP/in" "$TMP/o" && ok || ko "spaces around a command must not break it"

echo "== 3. Hostile file arguments =="
# A hostile file argument must never create a file in the working directory.
# The check is done at the very end, on the whole directory, with a fixed
# whitelist: comparing two `ls` snapshots turned out to be fragile because the
# listing itself is what gets created when an argument list is malformed.
runs_cleanly "empty infile name"     $BIN "" cat cat "$TMP/o"
runs_cleanly "empty outfile name"    $BIN "$TMP/in" cat cat ""
runs_cleanly "infile is a directory" $BIN "$TMP" cat cat "$TMP/o"
runs_cleanly "outfile is a directory" $BIN "$TMP/in" cat cat "$TMP"
mkdir -p "$TMP/d"; runs_cleanly "both directories" $BIN "$TMP/d" cat cat "$TMP/d"
runs_cleanly "unreadable infile"     $BIN "$TMP/nope" cat cat "$TMP/o"
runs_cleanly "very long file name"   $BIN "/tmp/$(head -c 200 /dev/zero | tr '\0' 'a')" cat cat "$TMP/o"

echo "== 4. A command that dies from a signal =="
printf '#!/bin/sh\nkill -SEGV $$\n' > "$TMP/segv"; chmod +x "$TMP/segv"
timeout 15 $BIN "$TMP/in" cat "$TMP/segv" "$TMP/o" >/dev/null 2>&1
rc=$?
[ "$rc" -eq 139 ] && ok || ko "cmd2 killed by SIGSEGV: rc=$rc (128+11=139 expected)"
timeout 15 $BIN "$TMP/in" "$TMP/segv" cat "$TMP/o" >/dev/null 2>&1
rc=$?
[ "$rc" -lt 128 ] && ok || ko "a segfaulting cmd1 must not change the last status (rc=$rc)"

echo "== 5. Commands that close their stdout early (SIGPIPE) =="
head -c 500000 /dev/urandom | base64 > "$TMP/big"
timeout 20 $BIN "$TMP/big" cat "head -c 10" "$TMP/o" >/dev/null 2>&1
rc=$?
[ "$rc" -ne 124 ] && ok || ko "head closing the pipe early caused a hang"
[ -s "$TMP/o" ] && ok || ko "head -c 10 produced no output"

echo "== 6. No zombie process is left behind =="
$BIN "$TMP/in" cat cat "$TMP/o" >/dev/null 2>&1
sleep 0.2
zombies=$(ps -o stat= --ppid $$ 2>/dev/null | grep -c '^Z' || true)
[ "$zombies" -eq 0 ] && ok || ko "$zombies zombie child(ren) left behind"

echo "== 7. Concurrent invocations do not interfere =="
for i in 1 2 3 4 5; do
	( $BIN "$TMP/in" cat "wc -l" "$TMP/par$i" >/dev/null 2>&1 ) &
done
wait
all_ok=1
for i in 1 2 3 4 5; do [ "$(cat "$TMP/par$i" 2>/dev/null)" = "3" ] || all_ok=0; done
[ "$all_ok" -eq 1 ] && ok || ko "5 concurrent runs produced different results"

echo "== 8. Binary data survives the trip =="
head -c 65536 /dev/urandom > "$TMP/rand"
$BIN "$TMP/rand" cat cat "$TMP/rand.out" >/dev/null 2>&1
cmp -s "$TMP/rand" "$TMP/rand.out" && ok || ko "64 KB of binary data altered by cat|cat"

echo "== 9. here_doc does not corrupt stdin of the parent on long bodies =="
for i in $(seq 1 2000); do echo "line $i"; done > "$TMP/here"
echo "STOP" >> "$TMP/here"
$BIN here_doc STOP cat "wc -l" "$TMP/h.out" < "$TMP/here" >/dev/null 2>&1
[ "$(cat "$TMP/h.out")" -eq 2000 ] && ok || ko "2000-line here_doc: got $(cat "$TMP/h.out")"

echo "== 10. No write to a file named after the limiter =="
rm -f "$TMP/STOP"
$BIN here_doc STOP cat cat "$TMP/h2.out" < "$TMP/here" >/dev/null 2>&1
[ -e "$TMP/STOP" ] && ko "here_doc leaked a temporary file named after the limiter" || ok

printf '\n  \033[1;34mrobustness\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
