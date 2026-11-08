#!/bin/bash
# =============================================================================
#  tests/lib/assert.sh — the tiny assertion library used by tests/cases/*.sh
# =============================================================================
#  Sourced, never executed.  Each case file prints its own verdicts and calls
#  assert_summary at the end; the orchestrator (tests/run_tests.sh) then reads
#  the exit status.
#
#    assert_ok    <label> <command...>   the command must succeed
#    assert_fail  <label> <reason>       an unconditional failure
#    assert_eq    <label> <expected> <got>
#    assert_ne    <label> <unexpected> <got>
#    assert_in    <label> <needle> <haystack>
# =============================================================================

C_OK=$'\033[1;32m'
C_ERR=$'\033[1;31m'
C_INFO=$'\033[1;34m'
C_WARN=$'\033[1;33m'
C_OFF=$'\033[0m'

ASSERT_PASS=0
ASSERT_FAIL=0

_assert_pass() {
	ASSERT_PASS=$((ASSERT_PASS + 1))
	printf '  %sPASS%s %s\n' "$C_OK" "$C_OFF" "$1"
}

_assert_fail() {
	ASSERT_FAIL=$((ASSERT_FAIL + 1))
	printf '  %sFAIL%s %s\n' "$C_ERR" "$C_OFF" "$1"
	[ -n "${2:-}" ] && printf '       %s\n' "$2"
}

assert_ok() {
	local label="$1"; shift
	local out
	if out=$("$@" 2>&1); then
		_assert_pass "$label"
	else
		_assert_fail "$label" "$(printf '%s' "$out" | head -3 | tr '\n' ' ')"
	fi
}

assert_fail() {
	_assert_fail "$1" "${2:-}"
}

assert_eq() {
	local label="$1" expected="$2" got="$3"
	if [ "$expected" = "$got" ]; then
		_assert_pass "$label"
	else
		_assert_fail "$label" "expected [$expected], got [$got]"
	fi
}

assert_ne() {
	local label="$1" unexpected="$2" got="$3"
	if [ "$unexpected" != "$got" ]; then
		_assert_pass "$label"
	else
		_assert_fail "$label" "value should differ from [$unexpected]"
	fi
}

assert_in() {
	local label="$1" needle="$2" haystack="$3"
	case "$haystack" in
	*"$needle"*) _assert_pass "$label" ;;
	*)           _assert_fail "$label" "[$needle] not found in the output" ;;
	esac
}

assert_summary() {
	local name="${1:-case}"
	printf '\n  %s%s%s: %d passed, %d failed\n' \
		"$C_INFO" "$name" "$C_OFF" "$ASSERT_PASS" "$ASSERT_FAIL"
	[ "$ASSERT_FAIL" -eq 0 ]
}
