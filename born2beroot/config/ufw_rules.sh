#!/bin/bash
# =============================================================================
#  ufw_rules.sh — the exact firewall policy of the Born2beRoot VM
# =============================================================================
#  Mandatory part: UFW active, *only* port 4242 open.
#  Bonus part:     the extra services open their own ports (chapter VII
#                  explicitly allows opening more ports for the bonus, as long
#                  as the UFW rules are adapted accordingly).
#
#  Executed by scripts/setup/40-ufw.sh, which calls "ufw --force reset" first
#  so the resulting table is exactly the one described below.
# =============================================================================

set -e

# Default deny: nothing comes in, everything goes out.
ufw default deny incoming
ufw default allow outgoing

# --- Mandatory ---------------------------------------------------------------
ufw allow 4242/tcp comment 'SSH on port 4242 (mandatory part)'

# --- Bonus services ----------------------------------------------------------
# Guarded by B2BR_BONUS: the mandatory part must leave *only* 4242 open
# ("leave only port 4242 open"), so these rules are added by the bonus scripts
# and by nothing else.  40-ufw.sh exports B2BR_BONUS=1 only when setup.sh was
# run with --bonus.
if [ "${B2BR_BONUS:-0}" = "1" ]; then
	ufw allow 80/tcp   comment 'lighttpd + WordPress (bonus)'
	ufw allow 21/tcp   comment 'vsftpd control channel (bonus)'
	ufw allow 40000:40010/tcp comment 'vsftpd passive ports (bonus)'
	# MariaDB: the port is opened only on the loopback interface, so the
	# database is unreachable from the network even though the rule exists.
	ufw allow in on lo to any port 3306 proto tcp comment 'MariaDB loopback only'
fi

# --- Activate ----------------------------------------------------------------
ufw --force enable

# The reference output of the subject is:
#   Status: active
#   To            Action   From
#   4242          ALLOW    Anywhere
#   4242 (v6)     ALLOW    Anywhere (v6)
ufw status verbose
