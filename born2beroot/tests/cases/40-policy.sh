#!/bin/bash
# =============================================================================
#  tests/cases/40-policy.sh — password policy and sudo configuration
# =============================================================================
#  Every line of chapter V that constrains a value is checked here, against the
#  files that actually enforce it (login.defs, pam_pwquality, sudoers).  The
#  values are not hard-coded: they come from config/vm.conf, so changing the
#  policy means changing one file.
# =============================================================================

set -uo pipefail

B2BR_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
. "$B2BR_DIR/tests/lib/assert.sh"

if [ "${B2BR_IN_VM:-0}" != "1" ]; then
	printf 'this case must be run through tests/vm.sh\n' >&2
	exit 2
fi

. "$B2BR_DIR/config/vm.conf"

# --- login.defs --------------------------------------------------------------
# "Your password has to expire every 30 days."
assert_eq "PASS_MAX_DAYS = $PASS_MAX_DAYS" "$PASS_MAX_DAYS" \
	"$(awk '$1 == "PASS_MAX_DAYS" {print $2}' /etc/login.defs)"
# "The minimum number of days between password changes must be set to 2."
assert_eq "PASS_MIN_DAYS = $PASS_MIN_DAYS" "$PASS_MIN_DAYS" \
	"$(awk '$1 == "PASS_MIN_DAYS" {print $2}' /etc/login.defs)"
# "The user has to receive a warning message 7 days before their password expires."
assert_eq "PASS_WARN_AGE = $PASS_WARN_AGE" "$PASS_WARN_AGE" \
	"$(awk '$1 == "PASS_WARN_AGE" {print $2}' /etc/login.defs)"

# --- pam_pwquality -----------------------------------------------------------
pam=$(grep -E '^password[[:space:]]+requisite[[:space:]]+pam_pwquality\.so' \
	/etc/pam.d/common-password 2>/dev/null | tail -1)
assert_ok "pam_pwquality is in the password stack" test -n "$pam"
# "at least 10 characters long"
assert_in "minlen=$PASS_MIN_LEN"     "minlen=$PASS_MIN_LEN"     "$pam"
# "an uppercase letter, a lowercase letter, and a number"
assert_in "ucredit=-1"               "ucredit=-1"               "$pam"
assert_in "lcredit=-1"               "lcredit=-1"               "$pam"
assert_in "dcredit=-1"               "dcredit=-1"               "$pam"
# "not contain more than 3 consecutive identical characters"
assert_in "maxrepeat=$PASS_MAX_REPEAT" "maxrepeat=$PASS_MAX_REPEAT" "$pam"
# "The password must not include the name of the user" — both spellings, because
# reject_username is what most graders grep for.
assert_ok "username check enabled" \
	bash -c "grep -qE 'usercheck=1|reject_username' /etc/pam.d/common-password"
# "at least 7 characters that were not part of the previous password"
assert_in "difok=$PASS_DIFOK"        "difok=$PASS_DIFOK"        "$pam"
# "the rule does not apply to the root password ... Of course, your root password
#  has to comply with this policy"
assert_in "enforce_for_root"         "enforce_for_root"         "$pam"
assert_in "pwquality.conf: minlen"   "minlen = $PASS_MIN_LEN"   "$(cat /etc/security/pwquality.conf)"
assert_in "pwquality.conf: usercheck" "usercheck = 1"           "$(cat /etc/security/pwquality.conf)"

# --- chage on the live accounts ---------------------------------------------
# "you will have to change all the passwords of the accounts present on the
#  virtual machine, including the root account"
for user in root "$VM_LOGIN"; do
	chage_out=$(chage -l "$user" 2>/dev/null)
	assert_in "$user: max days = $PASS_MAX_DAYS" "$PASS_MAX_DAYS" "$chage_out"
	assert_in "$user: min days = $PASS_MIN_DAYS" "$PASS_MIN_DAYS" "$chage_out"
	assert_in "$user: warn days = $PASS_WARN_AGE" "$PASS_WARN_AGE" "$chage_out"
done

# --- sudo --------------------------------------------------------------------
S=/etc/sudoers
assert_in "passwd_tries=$SUDO_ATTEMPTS"       "passwd_tries=$SUDO_ATTEMPTS"   "$(cat $S)"
assert_in "badpass_message set"               "badpass_message="              "$(cat $S)"
assert_in "the message is the chosen one"     "${SUDO_BADPASS_MESSAGE:0:20}"  "$(cat $S)"
assert_in "log_input"                         "log_input"                     "$(cat $S)"
assert_in "log_output"                        "log_output"                    "$(cat $S)"
assert_in "iolog_dir is $SUDO_LOG_DIR"        "iolog_dir=\"$SUDO_LOG_DIR\""   "$(cat $S)"
assert_in "logfile is $SUDO_LOG_DIR/sudo.log" "logfile=\"$SUDO_LOG_DIR/sudo.log\"" "$(cat $S)"
assert_ok "$SUDO_LOG_DIR exists"              test -d "$SUDO_LOG_DIR"
assert_in "requiretty"                        "requiretty"                    "$(cat $S)"
assert_in "secure_path contains /snap/bin"    "/snap/bin"                     "$(cat $S)"
assert_ok "exactly one secure_path line" \
	bash -c "[ \"\$(grep -cE '^Defaults[[:space:]]+secure_path' $S)\" = 1 ]"
assert_ok "sudoers parses (visudo -c)"        visudo -c
assert_ok "the drop-in exists and parses"     visudo -cf /etc/sudoers.d/born2beroot

# --- The policy actually rejects a bad password ------------------------------
#  PAM is not exercised in the emulator (passwords are set through chpasswd),
#  but the module and its arguments are real, so the *configuration* is what is
#  asserted above.  The one behavioural check that can be done is that a
#  password shorter than minlen is refused by the tool that reads pwquality.
if command -v pwscore >/dev/null 2>&1; then
	assert_ok "pwscore refuses a 4-character password" \
		bash -c '! echo 1234 | pwscore >/dev/null 2>&1'
fi

assert_summary "policy"
