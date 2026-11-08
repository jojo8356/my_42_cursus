#!/usr/bin/env bash
# =============================================================================
#  run_community.sh — run the community testers against the emulated VM
# =============================================================================
#  The community testers under tests/community/ are kept **verbatim** (only
#  their file names were normalised).  They were written to run as root on a
#  real Born2beRoot virtual machine: they call lsblk, ufw, crontab, lshw,
#  apt-get...  tests/vm.sh provides exactly that environment, so they work
#  unmodified, which is the strongest possible check that they are faithful.
#
#  Usage: tests/run_community.sh [--bonus] [--verbose] [tester...]
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
IMAGE="${B2BR_IMAGE:-/tmp/b2br-community-vm}"
BONUS=0
VERBOSE=0
WANT=()

for arg in "$@"; do
	case "$arg" in
	--bonus)   BONUS=1 ;;
	--verbose) VERBOSE=1 ;;
	*)         WANT+=("$arg") ;;
	esac
done

. "$B2BR_DIR/config/vm.conf"

pass=0
fail=0
skip=0

say()  { printf '%s\n' "$*"; }
head_() { printf '\n\033[1;34m== %s ==\033[0m\n' "$*"; }

# --- 1. Provision a machine the testers can inspect -------------------------
head_ "Provisioning the emulated VM ($IMAGE)"
if [ -d "$IMAGE" ]; then
	say "  reusing the existing image (delete $IMAGE to rebuild)"
else
	bash "$B2BR_DIR/tests/vm.sh" --snapshot "$IMAGE" -- \
		"bash /opt/born2beroot/scripts/setup.sh $([ "$BONUS" = 1 ] && echo --bonus) >/dev/null 2>&1" \
		|| { say "  provisioning failed"; exit 1; }
	say "  done"
fi

# --- 2. Run each tester ------------------------------------------------------
# Each tester prints [OK] / [GOOD] / [OK ] on success and [KO] / [FAILED] /
# [FAIL] otherwise.  The exit status is ignored: what matters is the verdict
# of every individual check, exactly like a human evaluator would read it.
run_tester() {
	local name="$1" file="$2" prepare="$3"; shift 3
	local log rc ok ko extra cmd

	# Remaining arguments are appended to the tester's own command line.
	# The extra arguments are passed verbatim; each one is quoted so that a
	# value containing spaces (a path, a message) survives the trip through the
	# emulator's command string.
	extra=""
	local a
	for a in "$@"; do extra="$extra ${a}"; done

	head_ "$name"
	log=$(mktemp)
	# The testers assume they were started through sudo by the evaluator, so
	# $SUDO_USER is the login they compare the hostname against.  These scripts
	# run as root directly, hence the explicit export.
	cmd="export SUDO_USER='$VM_LOGIN' TERM=xterm-256color; cd /root"
	[ -n "$prepare" ] && cmd="$cmd; $prepare"
	cmd="$cmd; bash /opt/born2beroot/tests/community/$file$extra 2>&1"

	bash "$B2BR_DIR/tests/vm.sh" --run "$IMAGE" -- "$cmd" > "$log" 2>&1
	rc=$?
	# Verdict notations differ per tester: [OK] [GOOD] [+] on one side,
	# [KO] [FAILED] [-] on the other.  The colour codes around them are stripped
	# first, otherwise "[\x1b[..mKO]" never matches a plain "[KO]".
	local clean
	clean=$(mktemp)
	# The legend at the top of some testers prints one line per possible verdict
	# ("[KO] Test failed to execute.").  Those lines are documentation, not
	# results, so they are removed before the verdicts are counted.
	sed 's/\x1b\[[0-9;]*m//g' "$log" \
		| grep -vE '^\[KO\][[:space:]]+Test (might have failed|Failed on basic|failed to execute)' \
		| grep -vE '^\[OK\][[:space:]]+Test (checks out|probably checks out)' > "$clean"
	ok=$(grep -c -E '\[ ?OK ?\]|\[GOOD\]|\[ ?\+ ?\]' "$clean")
	ko=$(grep -c -E '\[KO\]|\[FAILED\]|\[ ?- ?\]' "$clean")
	[ "$VERBOSE" = "1" ] && cat "$log"

	if [ "$ok" = "0" ] && [ "$ko" = "0" ]; then
		skip=$((skip + 1))
		printf '  %sSKIP%s %-34s no verdict found (rc=%d)\n' \
			$'\033[1;33m' $'\033[0m' "$name" "$rc"
		printf '        log: %s\n' "$log"
		return
	fi
	if [ "$ko" = "0" ]; then
		pass=$((pass + 1))
		printf '  %sPASS%s %-34s %d/%d checks\n' \
			$'\033[1;32m' $'\033[0m' "$name" "$ok" "$((ok + ko))"
		rm -f "$log"
	else
		fail=$((fail + 1))
		printf '  %sFAIL%s %-34s %d KO / %d OK\n' \
			$'\033[1;31m' $'\033[0m' "$name" "$ko" "$ok"
		printf '        log: %s\n' "$log"
		if [ "$VERBOSE" != "1" ]; then
			grep -E '\[KO\]|\[FAILED\]|\[ ?- ?\]' "$clean" | head -8 | sed 's/^/        /'
		fi
		rm -f "$clean"
	fi
}

# The testers were written for different assumptions:
#  * gemartin99 / highrulez / pandashaly expect to run as root and inspect the
#    live machine: no preparation needed.
#  * othello42 can be driven from the hypervisor through ssh, which is not
#    available here, so it is run directly inside the machine.
#  * pixailz takes the login as an argument and needs the monitoring script
#    path; it also reads "who" to build its report.
PIXAILZ_PREP="printf '%s\n' '$VM_LOGIN  tty1  2026-11-08 19:00' > /var/lib/b2br/sessions; printf '%s\n' '$VM_LOGIN  pts/0  2026-11-08 19:10 (10.0.2.2)' >> /var/lib/b2br/sessions"

run_one() {
	local key="$1"
	case "$key" in
	gemartin99)  run_tester "gemartin99/Born2beroot-Tester"      gemartin99_Test.sh "" ;;
	highrulez)   run_tester "highrulez/Born2BeRoot-Tester"       highrulez_test.sh "" ;;
	pandashaly)  run_tester "pandashaly/Born2BeRoot_tester"      pandashaly_tester.sh "" ;;
	othello42)   run_tester "Othello42/Born2BeRoot-tester"       othello42_debian_test.sh "" "$VM_LOGIN" ;;
	pixailz)     run_tester "Pixailz/Born2BeRootTester"          pixailz_grade_me.sh \
		"cp -f /opt/born2beroot/tests/community/pixailz_dot_s.sh /root/.s && chmod 0755 /root/.s" \
		"-u $VM_LOGIN -m $MONITORING_SCRIPT" ;;
	*)           say "  unknown tester: $key" ;;
	esac
}

ALL=(gemartin99 highrulez pandashaly othello42 pixailz)
if [ "${#WANT[@]}" -eq 0 ]; then
	WANT=("${ALL[@]}")
fi
for t in "${WANT[@]}"; do
	run_one "$t"
done

# --- 3. Summary -------------------------------------------------------------
printf '\n============================================================\n'
printf ' Community testers: %d passed, %d failed, %d skipped\n' \
	"$pass" "$fail" "$skip"
printf '============================================================\n'
[ "$fail" -eq 0 ]
