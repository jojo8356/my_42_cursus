#!/bin/bash
# ---------------------------------------------------------------------------
#  push_swap -- test orchestrator
#
#  Six layers, each answering a different question:
#
#    static      does the source obey the subject and the Norm?
#    mandatory   does the program do what the subject says, clause by clause?
#    benchmark   does it hit the numbers of chapter VI.4 (700 and 5500)?
#    bonus       does the checker behave like the official one?
#    robustness  does it survive hostile arguments and hostile streams?
#    memory      does it leak, or overflow a buffer, on any path?
#    community   do the third-party testers agree?
#
#  Usage:
#    ./run_tests.sh                 everything
#    ./run_tests.sh --quick         skip the sanitizer rebuild and the community
#    ./run_tests.sh --no-community
#    ./run_tests.sh --layer bonus
# ---------------------------------------------------------------------------
set -uo pipefail

ROOT=$(cd "$(dirname "$0")" && pwd)
cd "$ROOT" || exit 1

BOLD='\033[1m'
GREEN='\033[32m'
RED='\033[31m'
YELLOW='\033[33m'
BLUE='\033[34m'
DIM='\033[2m'
RESET='\033[0m'

QUICK=0
COMMUNITY=1
ONLY=""
SAMPLES=${SAMPLES:-10}

while [ $# -gt 0 ]; do
	case "$1" in
		--quick) QUICK=1; COMMUNITY=0 ;;
		--no-community) COMMUNITY=0 ;;
		--layer) shift; ONLY=${1:-} ;;
		-h|--help)
			sed -n '2,22p' "$0" | sed 's/^# \{0,1\}//'
			exit 0 ;;
		*) printf 'unknown option: %s\n' "$1"; exit 2 ;;
	esac
	shift
done

PASSED=0
FAILED=0
SKIPPED=0
declare -a SUMMARY=()

# --------------------------------------------------------------------------
# banner / helpers
# --------------------------------------------------------------------------
printf '\n%b%b push_swap — test suite %b\n' "$BOLD" "$BLUE" "$RESET"
printf '%b%s%b\n\n' "$DIM" "$(printf '%.0s─' $(seq 1 72))" "$RESET"

clean() {
	sed 's/\x1b\[[0-9;]*m//g'
}

layer_wanted() {
	[ -z "$ONLY" ] || [ "$ONLY" = "$1" ]
}

run_layer() {
	local name=$1 script=$2
	layer_wanted "$name" || return 0
	printf '%b▶ %s%b\n' "$BOLD" "$name" "$RESET"
	local out rc
	out=$(SAMPLES="$SAMPLES" bash "$script" 2>&1)
	rc=$?
	printf '%s\n' "$out" | sed -n '/==/p;/\[KO\]/p;/limit/p;/avg/p'
	if [ "$rc" -eq 0 ]; then
		local detail
		detail=$(printf '%s\n' "$out" | grep -E '^  .*(passed,|checks on)' | tail -1 | sed 's/^ *//' | sed 's/\x1b\[[0-9;]*m//g')
		printf '  %bPASS%b %-12s %b%s%b\n\n' "$GREEN" "$RESET" "$name" "$DIM" "$detail" "$RESET"
		PASSED=$((PASSED + 1))
		SUMMARY+=("PASS $name — $detail")
	else
		printf '  %bFAIL%b %-12s\n\n' "$RED" "$RESET" "$name"
		printf '%s\n' "$out" | grep -A3 '\[KO\]' | head -20 | sed 's/^/       /'
		FAILED=$((FAILED + 1))
		SUMMARY+=("FAIL $name")
	fi
}

skip_layer() {
	local name=$1 why=$2
	layer_wanted "$name" || return 0
	printf '%b▶ %s%b\n  %bSKIP%b %s\n\n' "$BOLD" "$name" "$RESET" "$YELLOW" "$RESET" "$why"
	SKIPPED=$((SKIPPED + 1))
	SUMMARY+=("SKIP $name — $why")
}

# --------------------------------------------------------------------------
# build first: several layers assume both binaries exist
# --------------------------------------------------------------------------
printf '%b▶ build%b\n' "$BOLD" "$RESET"
build_log=$(make -C "$ROOT" fclean >/dev/null 2>&1; make -C "$ROOT" 2>&1)
rc1=$?
bonus_log=$(make -C "$ROOT" bonus 2>&1)
rc2=$?
if [ "$rc1" -eq 0 ] && [ "$rc2" -eq 0 ] \
	&& [ -x "$ROOT/push_swap" ] && [ -x "$ROOT/checker" ]; then
	warn=$(printf '%s\n%s\n' "$build_log" "$bonus_log" | grep -ci warning)
	if [ "$warn" -eq 0 ]; then
		printf '  %bPASS%b %-12s %bboth binaries, 0 warning%b\n\n' \
			"$GREEN" "$RESET" "build" "$DIM" "$RESET"
		PASSED=$((PASSED + 1))
		SUMMARY+=("PASS build — both binaries, 0 warning")
	else
		printf '  %bFAIL%b %-12s %b%d warnings%b\n\n' "$RED" "$RESET" "build" "$DIM" "$warn" "$RESET"
		FAILED=$((FAILED + 1))
		SUMMARY+=("FAIL build — $warn warnings")
	fi
else
	printf '  %bFAIL%b %-12s\n' "$RED" "$RESET" "build"
	printf '%s\n' "$build_log" "$bonus_log" | tail -12 | sed 's/^/       /'
	FAILED=$((FAILED + 1))
	SUMMARY+=("FAIL build")
fi

# --------------------------------------------------------------------------
# the layers
# --------------------------------------------------------------------------
run_layer static tests/cases/00-static.sh
run_layer mandatory tests/cases/10-mandatory.sh
run_layer benchmark tests/cases/20-benchmark.sh
run_layer bonus tests/cases/30-bonus-checker.sh
run_layer robustness tests/cases/40-robustness.sh

if [ "$QUICK" -eq 1 ]; then
	skip_layer memory "--quick"
else
	run_layer memory tests/cases/50-memory.sh
fi

if [ "$COMMUNITY" -eq 0 ]; then
	skip_layer community "--quick or --no-community"
else
	layer_wanted community || true
	if layer_wanted community; then
		printf '%b▶ community%b\n' "$BOLD" "$RESET"
		out=$(bash "$ROOT/tests/community.sh" 2>&1)
		rc=$?
		printf '%s\n' "$out" | sed -n '/==/p;/\[KO\]/p;/PASS/p;/FAIL/p'
		if [ "$rc" -eq 0 ]; then
			printf '  %bPASS%b %-12s %bthird-party testers agree%b\n\n' \
				"$GREEN" "$RESET" "community" "$DIM" "$RESET"
			PASSED=$((PASSED + 1))
			SUMMARY+=("PASS community — third-party testers agree")
		else
			printf '  %bFAIL%b %-12s\n\n' "$RED" "$RESET" "community"
			FAILED=$((FAILED + 1))
			SUMMARY+=("FAIL community")
		fi
	fi
fi

# --------------------------------------------------------------------------
# summary
# --------------------------------------------------------------------------
printf '%b%s%b\n' "$DIM" "$(printf '%.0s─' $(seq 1 72))" "$RESET"
printf '%b  %d passed, %d failed, %d skipped%b\n\n' \
	"$BOLD" "$PASSED" "$FAILED" "$SKIPPED" "$RESET"

for line in "${SUMMARY[@]}"; do
	status=${line%% *}
	rest=${line#* }
	case "$status" in
		PASS) printf '  %b●%b %s\n' "$GREEN" "$RESET" "$rest" ;;
		FAIL) printf '  %b●%b %s\n' "$RED" "$RESET" "$rest" ;;
		SKIP) printf '  %b●%b %s (skipped)\n' "$YELLOW" "$RESET" "$rest" ;;
	esac
done

printf '\n'
[ "$FAILED" -eq 0 ]
