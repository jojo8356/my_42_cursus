#!/usr/bin/env bash
# Memory: "your program mustn't have memory leaks" (III.2).
#
# valgrind is used when it is installed; otherwise the suite falls back to
# AddressSanitizer + LeakSanitizer, which detects the same leaks and also
# catches out-of-bounds access. Both binaries (mandatory and bonus) are
# rebuilt with the sanitizer so that the check cannot silently test the
# optimised build, where a small leak can be optimised away.

set -uo pipefail
cd "$(dirname "$0")/../.." || exit 1

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
PASS=0
FAIL=0
ok() { PASS=$((PASS + 1)); }
ko() { FAIL=$((FAIL + 1)); printf '  \033[31m[KO]\033[0m %s\n' "$1"; }

CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address,undefined"
MAND="src/main.c src/pipex_error.c src/pipex_split.c src/pipex_path.c src/pipex_exec.c src/pipex_run.c"
BONS="src/main_bonus.c src/pipex_error.c src/pipex_split.c src/pipex_path.c src/pipex_exec.c src/bonus/pipex_run_bonus.c src/bonus/pipex_child_bonus.c src/bonus/pipex_here_doc_bonus.c"

echo "== 1. Sanitizer builds =="
if cc $CFLAGS -I include $MAND libft/libft.a -o "$TMP/pipex_m" 2>"$TMP/err"; then ok
else ko "sanitizer build of the mandatory part failed"; head -5 "$TMP/err"; fi
if cc $CFLAGS -I include $BONS libft/libft.a -o "$TMP/pipex_b" 2>"$TMP/err"; then ok
else ko "sanitizer build of the bonus part failed"; head -5 "$TMP/err"; fi
[ -x "$TMP/pipex_m" ] && [ -x "$TMP/pipex_b" ] || { printf '\n  memory: 0 passed, 2 failed\n'; exit 1; }

printf 'a1\nb2\na3\n' > "$TMP/in"
printf 'x\ny\nEND\n' > "$TMP/here"
export ASAN_OPTIONS=detect_leaks=1:abort_on_error=0:exitcode=99
export UBSAN_OPTIONS=halt_on_error=0:exitcode=98

# clean_run <label> <binary> <args...>  -- stdin can be redirected by the caller
clean_run() {
	local label=$1; shift
	local rc
	timeout 60 "$@" >/dev/null 2>"$TMP/log"
	rc=$?
	if [ "$rc" -eq 99 ]; then
		ko "$label: leak detected"
		grep -E "SUMMARY|leaked" "$TMP/log" | head -2
	elif [ "$rc" -eq 98 ]; then
		ko "$label: undefined behaviour"
		head -3 "$TMP/log"
	elif grep -qE "ERROR: (AddressSanitizer|LeakSanitizer)|runtime error" "$TMP/log"; then
		ko "$label: sanitizer report"
		head -3 "$TMP/log"
	else
		ok
	fi
}

echo "== 2. Mandatory part, success paths =="
clean_run "nominal pipeline"     "$TMP/pipex_m" "$TMP/in" "grep a" "wc -w" "$TMP/o"
clean_run "empty stream"         "$TMP/pipex_m" "$TMP/in" "grep zzz" "wc -l" "$TMP/o"
clean_run "90 KB through a pipe" "$TMP/pipex_m" /etc/services cat "wc -c" "$TMP/o"

echo "== 3. Mandatory part, error paths (where leaks hide) =="
clean_run "missing infile"       "$TMP/pipex_m" "$TMP/nope" cat cat "$TMP/o"
clean_run "unwritable outfile"   "$TMP/pipex_m" "$TMP/in" cat cat /nope/out
clean_run "unknown cmd1"         "$TMP/pipex_m" "$TMP/in" "not-a-command" cat "$TMP/o"
clean_run "unknown cmd2"         "$TMP/pipex_m" "$TMP/in" cat "not-a-command" "$TMP/o"
clean_run "wrong arg count"      "$TMP/pipex_m" "$TMP/in" cat
clean_run "empty command"        "$TMP/pipex_m" "$TMP/in" "" cat "$TMP/o"
clean_run "command with spaces"  "$TMP/pipex_m" "$TMP/in" "   " "  cat  " "$TMP/o"

echo "== 4. Bonus part, multiple pipes =="
clean_run "3 commands"           "$TMP/pipex_b" "$TMP/in" cat cat "wc -l" "$TMP/o"
clean_run "8 commands"           "$TMP/pipex_b" "$TMP/in" cat cat cat cat cat cat cat "wc -c" "$TMP/o"
clean_run "unknown in the middle" "$TMP/pipex_b" "$TMP/in" cat "not-a-command" cat "$TMP/o"
clean_run "missing infile (bonus)" "$TMP/pipex_b" "$TMP/nope" cat cat "$TMP/o"

echo "== 5. Bonus part, here_doc =="
clean_run "here_doc nominal"     "$TMP/pipex_b" here_doc END cat "wc -l" "$TMP/h" < "$TMP/here"
clean_run "here_doc no limiter"  "$TMP/pipex_b" here_doc END cat "wc -l" "$TMP/h" < /dev/null
clean_run "here_doc chain"       "$TMP/pipex_b" here_doc END grep x "tr a-z A-Z" cat "wc -c" "$TMP/h" < "$TMP/here"
clean_run "here_doc bad command" "$TMP/pipex_b" here_doc END "not-a-command" cat "$TMP/h" < "$TMP/here"

echo "== 6. No still-reachable block on the error path =="
# A leak that only happens when a command cannot be found is the most common
# pipex leak: px_split() allocates the argv and execve() never returns.
if command -v valgrind >/dev/null 2>&1; then
	valgrind --leak-check=full --error-exitcode=99 -q \
		./pipex "$TMP/in" "not-a-command" cat "$TMP/o" >/dev/null 2>&1
	[ $? -ne 99 ] && ok || ko "valgrind found a leak on the execve error path"
else
	ok
fi

printf '\n  \033[1;34mmemory\033[0m: %d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
