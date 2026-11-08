#!/usr/bin/env bash
# ============================================================================
#  pipex — full verification orchestrator (Subject v2, mandatory + bonus)
#
#  Layers, in the order a reviewer would follow:
#    1. static     42 headers, Norm basics, allowed functions, Makefile rules,
#                   the binary's imported symbols, no relink        (18 checks)
#    2. mandatory  differential tests against bash on the two commands (36)
#    3. bonus      N pipes and here_doc, compared with bash            (27)
#    4. robustness hostile arguments, signals, zombies, concurrency    (29)
#    5. memory     ASan/UBSan (or valgrind) on success and error paths (21)
#    6. community  three third-party testers, kept verbatim:
#                    vfurmane/pipex-tester   31 tests
#                    michmos/42_pipex_tester 28 tests
#                    Yoo0lh/pipex_tester_42  12 tests
#
#  Usage:
#     ./run_tests.sh              everything
#     ./run_tests.sh --quick      layers 1-4 only (no sanitizer rebuild)
#     ./run_tests.sh --no-community
#     ./run_tests.sh --layer X    run one layer: static|mandatory|bonus|
#                                 robustness|memory|community
# ============================================================================
set -uo pipefail
cd "$(dirname "$0")"

QUICK=0
COMMUNITY=1
ONLY=""
while [ $# -gt 0 ]; do
	case "$1" in
		--quick) QUICK=1 ;;
		--no-community) COMMUNITY=0 ;;
		--layer) shift; ONLY="${1:-}" ;;
		-h|--help) sed -n '3,22p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
		*) echo "unknown option: $1" >&2; exit 1 ;;
	esac
	shift
done

BOLD=$'\033[1m'; DIM=$'\033[2m'
GREEN=$'\033[32m'; RED=$'\033[31m'; YELLOW=$'\033[33m'; BLUE=$'\033[34m'
NC=$'\033[0m'

PASSED=0
FAILED=0
SKIPPED=0

want() { [ -z "$ONLY" ] || [ "$ONLY" = "$1" ]; }

# run_layer <name> <script> [args...]
run_layer() {
	local name=$1; shift
	[ -n "$ONLY" ] && [ "$ONLY" != "$name" ] && return 0
	printf '%s\n' "${BLUE}=== Layer: $name ===${NC}"
	local out rc
	out=$("$@" 2>&1)
	rc=$?
	printf '%s\n' "$out" | sed 's/^/    /'
	local line
	line=$(printf '%s\n' "$out" | grep -E '[a-z]+: [0-9]+ passed, [0-9]+ failed' | tail -1)
	if [ "$rc" -eq 0 ]; then
		PASSED=$((PASSED + 1))
		printf '  %sPASS%s %-20s %s\n' "$GREEN" "$NC" "$name" "$(printf '%s' "$line" | sed 's/^ *//; s/\x1b\[[0-9;]*m//g')"
	else
		FAILED=$((FAILED + 1))
		printf '  %sFAIL%s %-20s %s\n' "$RED" "$NC" "$name" "$(printf '%s' "$line" | sed 's/^ *//; s/\x1b\[[0-9;]*m//g')"
	fi
	printf '\n'
}

echo "============================================================"
echo "  pipex — full test suite"
echo "  $(cc --version | head -1)"
echo "============================================================"
echo

# ---------------------------------------------------------------------------
if want static; then
	printf '%s\n' "${BLUE}=== Layer: static ===${NC}"
	out=$(bash tests/cases/00-static.sh 2>&1); rc=$?
	printf '%s\n' "$out" | grep -E 'KO|passed' | sed 's/^/    /'
	line=$(printf '%s\n' "$out" | grep -E 'static: [0-9]+ passed' | tail -1 | sed 's/\x1b\[[0-9;]*m//g; s/^ *//')
	if [ "$rc" -eq 0 ]; then PASSED=$((PASSED + 1)); printf '  %sPASS%s %-20s %s\n\n' "$GREEN" "$NC" static "$line"
	else FAILED=$((FAILED + 1)); printf '  %sFAIL%s %-20s %s\n\n' "$RED" "$NC" static "$line"; fi
fi

if want mandatory; then
	run_layer mandatory bash tests/cases/10-mandatory.sh
fi
if want bonus; then
	run_layer bonus bash tests/cases/20-bonus.sh
fi
if want robustness; then
	run_layer robustness bash tests/cases/30-robustness.sh
fi
if [ "$QUICK" -eq 0 ] && want memory; then
	run_layer memory bash tests/cases/40-memory.sh
elif [ "$QUICK" -eq 1 ]; then
	SKIPPED=$((SKIPPED + 1))
	printf '  %sSKIP%s %-20s --quick\n\n' "$YELLOW" "$NC" memory
fi

# ---------------------------------------------------------------------------
# Community testers. Each one expects a different layout, so each is run from
# the directory it was written for. They are kept verbatim: the only thing
# adapted is the path to the project, which their own documentation says to
# edit ("ADJUST PATH TO PIPEX DIRECTORY IF NECESSARY").
community_vfurmane() {
	local d=tests/community/vfurmane
	cat > "$d/config.vars" <<-EOF
	PROJECT_DIRECTORY='../../..'
	CHECK_UPDATE=0
	EOF
	chmod +x "$d/config.vars"
	local out
	out=$(cd "$d" && timeout 600 ./run.sh 2>&1 | sed 's/\x1b\[[0-9;]*m//g')
	local ko
	ko=$(printf '%s\n' "$out" | grep -cE '\[KO\]' || true)
	local okc
	okc=$(printf '%s\n' "$out" | grep -cE '\[OK\]' || true)
	if [ "$ko" -eq 0 ] && [ "$okc" -gt 0 ]; then
		printf '  \033[1;34mvfurmane\033[0m: %d passed, 0 failed\n' "$okc"
		return 0
	fi
	printf '  \033[1;34mvfurmane\033[0m: %d passed, %d failed\n' "$okc" "$ko"
	printf '%s\n' "$out" | grep -E '\[KO\]' | head -10
	return 1
}

community_michmos() {
	local d=tests/community/michmos
	local out
	out=$(cd "$d" && timeout 900 ./run.sh 2>&1 | sed 's/\x1b\[[0-9;]*m//g')
	local ko okc
	ko=$(printf '%s\n' "$out" | grep -cE '\[KO\]' || true)
	okc=$(printf '%s\n' "$out" | sed -n 's/^# *[0-9]*:.*\[OK\].*$/x/p' | wc -l)
	if [ "$ko" -eq 0 ] && [ "$okc" -gt 0 ]; then
		printf '  \033[1;34mmichmos\033[0m: %d passed, 0 failed\n' "$okc"
		return 0
	fi
	printf '  \033[1;34mmichmos\033[0m: %d passed, %d failed\n' "$okc" "$ko"
	printf '%s\n' "$out" | grep -E '\[KO\]' | head -10
	return 1
}

community_yoo0lh() {
	# This tester expects to live *inside* the project (it calls ../pipex and
	# `make -C ../`). Copying it to a scratch directory keeps the repository
	# clean while running it exactly as its author intended.
	local d=tests/community/yoo0lh
	local scratch
	scratch=$(mktemp -d)
	mkdir -p "$scratch/pipex"
	cp -a "$d" "$scratch/pipex/tester"
	cp -a src include libft Makefile "$scratch/pipex/"
	cp -a "$d" "$scratch/pipex/tester"
	chmod +x "$scratch/pipex/tester/pipex_tester.sh"
	local out
	out=$(cd "$scratch/pipex/tester" && timeout 600 ./pipex_tester.sh a 2>&1 | sed 's/\x1b\[[0-9;]*m//g')
	local ko okc
	ko=$(printf '%s\n' "$out" | grep -cE '[0-9]+\. KO' || true)
	okc=$(printf '%s\n' "$out" | grep -cE '[0-9]+\. OK' || true)
	rm -rf "$scratch"
	if [ "$ko" -eq 0 ] && [ "$okc" -gt 0 ]; then
		printf '  \033[1;34myoo0lh\033[0m: %d passed, 0 failed\n' "$okc"
		return 0
	fi
	printf '  \033[1;34myoo0lh\033[0m: %d passed, %d failed\n' "$okc" "$ko"
	return 1
}

if [ "$COMMUNITY" -eq 1 ] && want community; then
	printf '%s\n' "${BLUE}=== Layer: community testers (kept verbatim) ===${NC}"
	bad=0
	out=$(community_vfurmane) || bad=1
	printf '%s\n' "$out" | sed 's/^/    /'
	out=$(community_michmos) || bad=1
	printf '%s\n' "$out" | sed 's/^/    /'
	out=$(community_yoo0lh) || bad=1
	printf '%s\n' "$out" | sed 's/^/    /'
	if [ "$bad" -eq 0 ]; then
		PASSED=$((PASSED + 1))
		printf '  %sPASS%s %-20s %s\n\n' "$GREEN" "$NC" community "3 testers, all checks green"
	else
		FAILED=$((FAILED + 1))
		printf '  %sFAIL%s %-20s %s\n\n' "$RED" "$NC" community "at least one tester failed"
	fi
elif [ "$COMMUNITY" -eq 0 ]; then
	SKIPPED=$((SKIPPED + 1))
	printf '  %sSKIP%s %-20s --no-community\n\n' "$YELLOW" "$NC" community
fi

# ---------------------------------------------------------------------------
printf '============================================================\n'
printf ' %d passed, %d failed, %d skipped\n' "$PASSED" "$FAILED" "$SKIPPED"
printf '============================================================\n'
[ "$FAILED" -eq 0 ]
