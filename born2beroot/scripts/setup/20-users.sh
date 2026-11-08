#!/bin/bash
# =============================================================================
#  20-users.sh — the 42 user, the user42 group, and password ageing
# =============================================================================
#  Subject v5.2, chapter V:
#    * "a user with your login as the username has to be present"
#    * "belongs to the user42 and sudo groups"
#    * during the defence, create a new user and assign it to a group
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../lib/common.sh"
load_config

require_root

# --- 1. user42 group ---------------------------------------------------------
if getent group "$EXTRA_GROUPS" >/dev/null 2>&1; then
	log_skip "group $EXTRA_GROUPS already exists"
elif [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would create group $EXTRA_GROUPS"
else
	groupadd "$EXTRA_GROUPS"
	log_ok "group $EXTRA_GROUPS created"
fi

# --- 2. The 42 user ----------------------------------------------------------
if id "$VM_LOGIN" >/dev/null 2>&1; then
	log_skip "user $VM_LOGIN already exists"
elif [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would create user $VM_LOGIN (groups: $USER_GROUPS)"
else
	# -m creates /home/<login>, -s gives a bash shell (the subject's script is
	# bash and the evaluator needs a working terminal), -G adds both groups.
	useradd -m -s /bin/bash -c "42 login" -G "$USER_GROUPS" "$VM_LOGIN"
	log_ok "user $VM_LOGIN created with groups $USER_GROUPS"
fi

# --- 3. Make sure both groups are present (idempotent) ----------------------
if [ "$B2BR_DRY_RUN" = "0" ]; then
	for g in $(echo "$USER_GROUPS" | tr ',' ' '); do
		if id -nG "$VM_LOGIN" 2>/dev/null | grep -qw "$g"; then
			log_skip "$VM_LOGIN already in group $g"
		else
			usermod -aG "$g" "$VM_LOGIN"
			log_ok "$VM_LOGIN added to group $g"
		fi
	done
fi

# --- 4. Password ageing for the user and for root ---------------------------
# The values also come from /etc/login.defs (see 50-password.sh); applying them
# to the existing accounts is required because login.defs only affects new ones.
for account in "$VM_LOGIN" root; do
	if [ "$B2BR_DRY_RUN" = "1" ]; then
		log_dry "would set chage -M $PASS_MAX_DAYS -m $PASS_MIN_DAYS -W $PASS_WARN_AGE on $account"
	elif id "$account" >/dev/null 2>&1; then
		chage -M "$PASS_MAX_DAYS" -m "$PASS_MIN_DAYS" -W "$PASS_WARN_AGE" "$account"
		log_ok "password ageing applied to $account ($PASS_MIN_DAYS/$PASS_MAX_DAYS/$PASS_WARN_AGE)"
	fi
done

# --- 5. The user must have a password -------------------------------
# The effective policy is what the evaluator checks; forcing a password change
# at first login is the cleanest way to satisfy it without storing a password
# in the repository.
if [ "$B2BR_DRY_RUN" = "0" ] && id "$VM_LOGIN" >/dev/null 2>&1; then
	# A freshly created account has no password ("!" in /etc/shadow) until
	# somebody sets one: that is expected on a new machine, so it is a notice,
	# not a failure.  A password *is* mandatory for the defence, which is why
	# the next step forces a change at first login.
	if [ -z "$(getent shadow "$VM_LOGIN" | cut -d: -f2)" ] \
		|| [ "$(getent shadow "$VM_LOGIN" | cut -d: -f2)" = "!" ]; then
		log_skip "user $VM_LOGIN has no password yet: run 'passwd $VM_LOGIN'"
	else
		log_ok "user $VM_LOGIN has a password"
	fi
	chage -d 0 "$VM_LOGIN" 2>/dev/null \
		&& log_ok "password change requested at next login for $VM_LOGIN" \
		|| true
fi

log_ok "20-users.sh done"
