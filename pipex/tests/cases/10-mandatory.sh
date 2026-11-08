#!/usr/bin/env bash
# Mandatory part: differential testing against bash.
#
# Every case is written twice -- once as `./pipex file1 cmd1 cmd2 file2` and
# once as the shell pipeline the subject says it must mimic -- then the two
# output files, the two exit statuses and the two stderr shapes are compared.
# Comparing against bash instead of hard-coded strings is what makes the suite
# meaningful: it is the reference the subject itself designates.

set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1

# Run in a scratch directory: the binary is exercised with malformed argument
# lists on purpose and the checkout must not collect their side effects.
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK" || exit 1

BIN="$ROOT/pipex"
# The mandatory suite must run against the *mandatory* binary: the bonus one
# accepts N commands and would make every argument-count check meaningless.
make -C "$ROOT" >/dev/null 2>&1 || { echo "make failed"; exit 1; }
if nm "$BIN" | grep -q px_run_bonus; then
	echo "the binary at ./pipex is the bonus build; rebuilding mandatory"
	make -C "$ROOT" re >/dev/null 2>&1 || exit 1
fi
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
PASS=0
FAIL=0

ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$1"; }

# run_case <label> <infile> <cmd1> <cmd2> <outfile> <shell-pipeline>
# The pipeline is given as the exact string bash must run for the reference.
run_case() {
	local label=$1 infile=$2 c1=$3 c2=$4 out=$5 ref=$6
	local my_rc sh_rc
	$BIN "$infile" "$c1" "$c2" "$out" >/dev/null 2>"$TMP/e1"
	my_rc=$?
	eval "$ref" >/dev/null 2>"$TMP/e2"
	sh_rc=$?
	if ! cmp -s "$out" "$TMP/ref"; then
		ko "$label: output differs"
		diff -u "$out" "$TMP/ref" 2>/dev/null | head -5
	elif [ "$my_rc" -ne "$sh_rc" ]; then
		ko "$label: exit status $my_rc, bash says $sh_rc"
	elif ! grep -qiE "$ERR_RE" "$TMP/e1" 2>/dev/null && [ -s "$TMP/e2" ]; then
		ko "$label: stderr does not mention the failure"
	else
		ok
	fi
}

# Helper that runs both sides and stores the reference output in $TMP/ref.
cmp_case() {
	local label=$1; shift
	local args=("$@")
	local ref="${args[4]}"
	$BIN "${args[@]}" >/dev/null 2>"$TMP/e1"
	local my_rc=$?
	eval "$ref" >/dev/null 2>"$TMP/e2"
	local sh_rc=$?
	if ! cmp -s "${args[3]}" "$TMP/ref"; then
		ko "$label: output differs"
		diff -u "${args[3]}" "$TMP/ref" | head -6 >&2
	elif [ "$my_rc" -ne "$sh_rc" ]; then
		ko "$label: exit $my_rc, bash $sh_rc"
	else
		ok
	fi
}

printf 'a1\nb2\na3\nc4\n'            > "$TMP/in1"
printf 'one two three\nfour five\n'  > "$TMP/in2"
: > "$TMP/empty"
head -c 100000 /dev/urandom | base64 > "$TMP/big"

echo "== 1. The two examples of the subject (III.1) =="
$BIN "$TMP/in1" "grep a" "wc -w" "$TMP/o1" >/dev/null 2>&1
/usr/bin/grep a < "$TMP/in1" | wc -w > "$TMP/ref" 2>/dev/null
{ [ "$(cat "$TMP/o1")" = "$(cat "$TMP/ref")" ] && ok; } || ko "grep a | wc -w"
$BIN "$TMP/in1" "ls -l" "wc -l" "$TMP/o2" >/dev/null 2>&1
ls -l | wc -l > "$TMP/ref" 2>/dev/null
{ [ "$(cat "$TMP/o2")" = "$(cat "$TMP/ref")" ] && ok; } || ko "ls -l | wc -l"

echo "== 2. Byte-exact output on ordinary pipelines =="
for spec in "cat:cat" "cat:sort" "grep[a1]:wc[ -l]" "head[ -2]:wc[ -c]" \
            "tr[a-z]:sort[ -u]" "sort:uniq" "cat:tail[ -n1]"; do
	c1=${spec%%:*}; c2=${spec#*:}
	c1=${c1//[/ }; c2=${c2//[/ }
	$BIN "$TMP/in2" "$c1" "$c2" "$TMP/out" >/dev/null 2>&1
	eval "< '$TMP/in2' $c1 | $c2 > '$TMP/ref'" 2>/dev/null
	if cmp -s "$TMP/out" "$TMP/ref"; then ok; else ko "output differs for '$c1 | $c2'"; fi
done

echo "== 3. Large input (pipe buffer must not deadlock) =="
$BIN "$TMP/big" cat "wc -c" "$TMP/obig" >/dev/null 2>&1
[ "$(cat "$TMP/obig")" -eq "$(wc -c < "$TMP/big")" ] && ok || ko "100 KB through one pipe"
$BIN "$TMP/big" cat cat "$TMP/obig2" >/dev/null 2>&1
cmp -s "$TMP/big" "$TMP/obig2" && ok || ko "byte-perfect copy of 100 KB"

echo "== 4. Reductions and empty streams =="
$BIN "$TMP/empty" cat "wc -c" "$TMP/oe" >/dev/null 2>&1
[ "$(cat "$TMP/oe")" -eq 0 ] && ok || ko "empty infile gives 0 bytes"
$BIN "$TMP/in1" "grep zzz" "wc -l" "$TMP/oz" >/dev/null 2>&1
[ "$(cat "$TMP/oz")" -eq 0 ] && ok || ko "no match gives 0 lines"

echo "== 5. Exit status is the one of the last command =="
for code in 0 1 5 42; do
	cat > "$TMP/exit$code" <<-EOF
	#!/bin/sh
	exit $code
	EOF
	chmod +x "$TMP/exit$code"
	$BIN "$TMP/in1" cat "$TMP/exit$code" "$TMP/o" >/dev/null 2>&1
	rc=$?
	[ "$rc" -eq "$code" ] && ok || ko "cmd2 exits $code, pipex returned $rc"
done

echo "== 6. A failing cmd1 does not stop cmd2 (bash has no pipefail) =="
$BIN "$TMP/in1" "$TMP/does-not-exist" "wc -l" "$TMP/of1" >/dev/null 2>&1
rc=$?
[ "$(cat "$TMP/of1")" -eq 0 ] && [ "$rc" -eq 0 ] && ok \
	|| ko "failing cmd1: outfile=$(cat "$TMP/of1") rc=$rc (expected 0/0)"

echo "== 7. Missing infile: cmd2 still runs on an empty stream =="
printf 'PREVIOUS\n' > "$TMP/of2"
$BIN "$TMP/nope" cat "wc -l" "$TMP/of2" >/dev/null 2>&1
rc=$?
[ "$(cat "$TMP/of2")" = "0" ] && [ "$rc" -eq 0 ] && ok \
	|| ko "missing infile: outfile='$(cat "$TMP/of2")' rc=$rc (bash gives '0' and 0)"
$BIN "$TMP/nope" cat cat "$TMP/of3" 2>"$TMP/e" >/dev/null
grep -q "No such file" "$TMP/e" && ok || ko "missing infile: no error message"

echo "== 8. Unwritable outfile: cmd1 still runs, error reported =="
start=$(date +%s)
$BIN "$TMP/in1" "sleep 2" cat /nonexistent-dir/out 2>"$TMP/e" >/dev/null
rc=$?
elapsed=$(( $(date +%s) - start ))
grep -q "No such file" "$TMP/e" && ok || ko "unwritable outfile: no error message"
[ "$rc" -ne 0 ] && ok || ko "unwritable outfile: rc should be non-zero"
[ "$elapsed" -ge 2 ] && ok || ko "unwritable outfile: cmd1 was skipped (${elapsed}s)"

echo "== 9. Wrong number of arguments =="
for n in 0 1 2 3 6; do
	args=()
	for ((i = 0; i < n; i++)); do args+=("$TMP/x$i"); done
	$BIN "${args[@]}" >/dev/null 2>"$TMP/e"
	rc=$?
	[ "$rc" -ne 0 ] && [ "$rc" -lt 128 ] && grep -qi "usage" "$TMP/e" && ok \
		|| ko "$n arguments: rc=$rc, usage message missing"
done

echo "== 10. Commands that do not exist =="
$BIN "$TMP/in1" "definitely-not-a-command" cat "$TMP/o" 2>"$TMP/e" >/dev/null
grep -qi "command not found" "$TMP/e" && ok || ko "cmd1 missing: no 'command not found'"
$BIN "$TMP/in1" cat "definitely-not-a-command" "$TMP/o" 2>"$TMP/e" >/dev/null
rc=$?
grep -qi "command not found" "$TMP/e" && [ "$rc" -eq 127 ] && ok \
	|| ko "cmd2 missing: rc=$rc (expected 127) and 'command not found'"

echo "== 11. Absolute and relative paths as commands =="
$BIN "$TMP/in1" "/usr/bin/grep a1" "/usr/bin/wc -l" "$TMP/o" >/dev/null 2>&1
[ "$(cat "$TMP/o")" -eq 1 ] && ok || ko "absolute paths as commands (got '$(cat "$TMP/o")')"
$BIN "$TMP/in1" "./tests/../pipex-no" cat "$TMP/o" >/dev/null 2>&1
[ $? -eq 0 ] && ok || ko "relative non-existent command must not abort the chain"

echo "== 12. PATH is used, and the environment is inherited =="
mkdir -p "$TMP/bin"
printf '#!/bin/sh\necho from-my-path\n' > "$TMP/bin/mycmd"
chmod +x "$TMP/bin/mycmd"
PATH="$TMP/bin:$PATH" $BIN "$TMP/in1" mycmd cat "$TMP/o" >/dev/null 2>&1
grep -q "from-my-path" "$TMP/o" && ok || ko "PATH lookup failed"
MYVAR=hello PATH="$TMP/bin:$PATH" $BIN "$TMP/in1" "sh -c env" "grep MYVAR" "$TMP/o" >/dev/null 2>&1
grep -q "MYVAR=hello" "$TMP/o" && ok || ko "environment not passed to execve"

echo "== 13. The output file is truncated, not appended =="
printf 'OLD-CONTENT-THAT-MUST-GO\n' > "$TMP/ot"
$BIN "$TMP/in1" cat "head -1" "$TMP/ot" >/dev/null 2>&1
grep -q "OLD-CONTENT" "$TMP/ot" && ko "outfile was not truncated" || ok

echo "== 14. No file descriptor leak (classic pipex hang) =="
timeout 10 $BIN "$TMP/big" cat cat "$TMP/oleak" >/dev/null 2>&1
rc=$?
[ "$rc" -eq 0 ] && cmp -s "$TMP/big" "$TMP/oleak" && ok || ko "hang or wrong output (rc=$rc)"

printf '\n  \033[1;34mmandatory\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
