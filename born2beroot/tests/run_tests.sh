#!/usr/bin/env bash
# =============================================================================
#  tests/run_tests.sh — the test suite orchestrator
# =============================================================================
#  Three layers, from the cheapest to the most expensive:
#
#    1. static   — no machine needed: syntax, configuration, repository hygiene
#    2. cases    — one script per subject chapter, run inside the emulated VM
#    3. community— five third-party testers, kept verbatim, run on that same VM
#
#  Usage:
#      tests/run_tests.sh [--bonus] [--static] [--cases] [--community]
#                         [--keep] [--image DIR] [-v]
#
#  Options:
#      --bonus      install the bonus part before testing (WordPress, vsftpd…)
#      --static     only the static layer
#      --cases      only the per-requirement cases
#      --community  only the community testers
#      --keep       keep the emulated disk image after the run
#      --image DIR  use (and keep) a specific image directory
#      -v           verbose: print each case's full output
#
#  Exit status: 0 only if every layer passed.
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

BONUS=0
RUN_STATIC=0
RUN_CASES=0
RUN_COMMUNITY=0
KEEP=0
VERBOSE=0
IMAGE="${B2BR_IMAGE:-/tmp/b2br-tests-vm}"

while [ $# -gt 0 ]; do
	case "$1" in
	--bonus)     BONUS=1 ;;
	--static)    RUN_STATIC=1 ;;
	--cases)     RUN_CASES=1 ;;
	--community) RUN_COMMUNITY=1 ;;
	--keep)      KEEP=1 ;;
	--image)     IMAGE="$2"; shift ;;
	-v|--verbose) VERBOSE=1 ;;
	-h|--help)   sed -n '3,28p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
	*)           printf 'unknown option: %s\n' "$1" >&2; exit 2 ;;
	esac
	shift
done

if [ "$RUN_STATIC$RUN_CASES$RUN_COMMUNITY" = "000" ]; then
	RUN_STATIC=1; RUN_CASES=1; RUN_COMMUNITY=1
fi

C_OK=$'\033[1;32m'
C_ERR=$'\033[1;31m'
C_INFO=$'\033[1;34m'
C_WARN=$'\033[1;33m'
C_OFF=$'\033[0m'

LAYER_PASS=0
LAYER_FAIL=0
FAILED_LAYERS=()

head_() { printf '\n%s=== %s ===%s\n' "$C_INFO" "$*" "$C_OFF"; }
LAYER_SKIP=0
verdict() { # verdict <ok|ko|skip> <name> <detail>
	if [ "$1" = "skip" ]; then
		LAYER_SKIP=$((LAYER_SKIP + 1))
		printf ' %sSKIP%s %-52s %s\n' "$C_WARN" "$C_OFF" "$2" "${3:-}"
		return
	fi
	if [ "$1" = "ok" ]; then
		LAYER_PASS=$((LAYER_PASS + 1))
		printf ' %sPASS%s %-52s %s\n' "$C_OK" "$C_OFF" "$2" "${3:-}"
	else
		LAYER_FAIL=$((LAYER_FAIL + 1))
		FAILED_LAYERS+=("$2")
		printf ' %sFAIL%s %-52s %s\n' "$C_ERR" "$C_OFF" "$2" "${3:-}"
	fi
}

# --- 0. Preconditions --------------------------------------------------------
head_ "Preconditions"
if [ ! -f "$B2BR_DIR/config/vm.conf" ]; then
	printf 'config/vm.conf is missing: not in the project root?\n' >&2
	exit 1
fi
verdict ok "project root" "$B2BR_DIR"
verdict ok "configuration" "config/vm.conf"
if [ "$BONUS" = "1" ]; then verdict ok "mode" "mandatory + bonus"; else verdict ok "mode" "mandatory"; fi

# --- 1. Static layer ---------------------------------------------------------
if [ "$RUN_STATIC" = "1" ]; then
	head_ "Layer 1 — static checks (no virtual machine)"
	if [ "$VERBOSE" = "1" ]; then
		bash "$B2BR_DIR/tests/cases/00-static.sh"
	else
		bash "$B2BR_DIR/tests/cases/00-static.sh" > /tmp/b2br-static.$$ 2>&1
	fi
	rc=$?
	tail=$(grep -E 'static: [0-9]+ passed' /tmp/b2br-static.$$ 2>/dev/null | tail -1)
	[ "$VERBOSE" = "1" ] || { [ "$rc" -eq 0 ] || cat /tmp/b2br-static.$$; }
	rm -f /tmp/b2br-static.$$
	if [ "$rc" -eq 0 ]; then verdict ok "static checks" "${tail#  }"; else verdict ko "static checks" "${tail#  }"; fi
fi

# --- 2. Machine-backed layers ------------------------------------------------
if [ "$RUN_CASES" = "1" ] || [ "$RUN_COMMUNITY" = "1" ]; then
	head_ "Provisioning the emulated machine"
	if [ -f "$IMAGE/.ready" ]; then
		verdict ok "image" "$IMAGE (reused)"
	else
		rm -rf "$IMAGE"
		if bash "$B2BR_DIR/tests/vm.sh" --snapshot "$IMAGE" -- \
			"bash /opt/born2beroot/scripts/setup.sh $([ "$BONUS" = 1 ] && echo --bonus)" \
			> /tmp/b2br-prov.$$ 2>&1; then
			verdict ok "image" "$IMAGE (setup.sh $([ "$BONUS" = 1 ] && echo --bonus))"
		else
			verdict ko "image" "setup.sh failed — see /tmp/b2br-prov.$$"
			grep -E '\[FAIL\]' /tmp/b2br-prov.$$ | head -5 | sed 's/^/       /'
		fi
		rm -f /tmp/b2br-prov.$$
	fi
fi

if [ "$RUN_CASES" = "1" ]; then
	head_ "Layer 2 — requirement cases (inside the machine)"
	for case in "$B2BR_DIR"/tests/cases/[1-9]*.sh; do
		name=$(basename "$case" .sh)
		# The bonus case asserts that lighttpd, vsftpd, fail2ban and their
		# ports exist: on a mandatory-only machine there is nothing to check.
		if [ "${name#60-}" != "$name" ] && [ "$BONUS" = "0" ]; then
			verdict skip "$name" "requires --bonus"
			continue
		fi
		out=$(bash "$B2BR_DIR/tests/vm.sh" --run "$IMAGE" -- \
			"B2BR_IN_VM=1 B2BR_BONUS=$BONUS bash /opt/born2beroot/tests/cases/$name.sh" 2>&1)
		rc=$?
		[ "$VERBOSE" = "1" ] && printf '%s\n' "$out"
		# Each case ends with a line like "  identity: 21 passed, 0 failed".
		tail=$(printf '%s\n' "$out" | grep -E '[a-z]+: [0-9]+ passed, [0-9]+ failed' | tail -1)
		detail=$(printf '%s' "$tail" | sed 's/^ *//' | sed 's/\x1b\[[0-9;]*m//g')
		[ -n "$detail" ] || detail="(no summary — the case did not run)"
		if [ "$rc" -eq 0 ]; then
			verdict ok "$name" "$detail"
		else
			verdict ko "$name" "$detail"
			[ "$VERBOSE" = "1" ] || printf '%s\n' "$out" | grep -E 'FAIL' | head -6 | sed 's/^/       /'
		fi
	done
fi

if [ "$RUN_COMMUNITY" = "1" ]; then
	head_ "Layer 3 — community testers (kept verbatim)"
	if [ "$BONUS" = "0" ]; then
		# Two of the five explicitly check the bonus part (lighttpd on port 80,
		# vsftpd, the extra ports).  They are written for the complete project,
		# so they only make sense on a machine that has it.
		verdict skip "community testers" "requires --bonus (they check the bonus services)"
	else
	out=$(B2BR_IMAGE="$IMAGE" bash "$B2BR_DIR/tests/run_community.sh" \
		$([ "$BONUS" = 1 ] && echo --bonus) 2>&1)
	rc=$?
	[ "$VERBOSE" = "1" ] && printf '%s\n' "$out"
	printf '%s\n' "$out" | grep -E '^  (PASS|FAIL|SKIP)' | sed 's/^\s*/  /'
	tail=$(printf '%s\n' "$out" | grep -E 'Community testers:' | tail -1)
	if [ "$rc" -eq 0 ]; then verdict ok "community testers" "$(printf '%s' "$tail" | sed 's/^ *//')"
	else verdict ko "community testers" "$(printf '%s' "$tail" | sed 's/^ *//')"; fi
	fi
fi

# --- 3. Self-check on the machine -------------------------------------------
if [ "$RUN_CASES" = "1" ] || [ "$RUN_COMMUNITY" = "1" ]; then
	head_ "Project self-check (scripts/verify.sh)"
	out=$(bash "$B2BR_DIR/tests/vm.sh" --run "$IMAGE" -- \
		'bash /opt/born2beroot/scripts/verify.sh' 2>&1)
	rc=$?
	tail=$(printf '%s\n' "$out" | grep -E 'checks passed' | tail -1)
	if [ "$rc" -eq 0 ]; then
		verdict ok "verify.sh" "${tail# }"
	else
		verdict ko "verify.sh" "${tail# }"
		printf '%s\n' "$out" | grep -E '\[FAIL\]' | head -8 | sed 's/^/       /'
	fi
fi

# --- 4. Clean up -------------------------------------------------------------
if [ "$KEEP" = "0" ] && [ -z "${B2BR_IMAGE:-}" ] && [ "$RUN_CASES$RUN_COMMUNITY" != "00" ]; then
	rm -rf "$IMAGE"
fi

# --- 5. Summary --------------------------------------------------------------
printf '\n============================================================\n'
printf ' %d passed, %d failed, %d skipped\n' "$LAYER_PASS" "$LAYER_FAIL" "$LAYER_SKIP"
if [ "$LAYER_FAIL" -gt 0 ]; then
	printf ' failed: %s\n' "${FAILED_LAYERS[*]}"
fi
printf '============================================================\n'
[ "$LAYER_FAIL" -eq 0 ]
