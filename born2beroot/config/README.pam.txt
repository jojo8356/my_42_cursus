# =============================================================================
#  Note about /etc/login.defs and /etc/pam.d/common-password
# =============================================================================
#  Both files are *system* files: they ship with the distribution and contain
#  hundreds of lines that must not be replaced.  scripts/setup/50-password.sh
#  therefore edits them in place, idempotently, with set_kv:
#
#  /etc/login.defs
#      PASS_MAX_DAYS   30      ... password expires every 30 days
#      PASS_MIN_DAYS   2       ... minimum 2 days between two changes
#      PASS_WARN_AGE   7       ... warning 7 days before expiry
#      ENCRYPT_METHOD  SHA512  ... modern hashing, not DES
#
#  /etc/pam.d/common-password
#      password requisite pam_pwquality.so retry=3 minlen=10 ucredit=-1 \
#          lcredit=-1 dcredit=-1 maxrepeat=3 difok=7 enforce_for_root \
#          usercheck=1
#      password [success=1 default=ignore] pam_unix.so obscure use_authtok \
#          try_first_pass yescrypt
#
#  /etc/pam.d/common-auth  (3 sudo attempts are enforced by sudo itself, see
#  /etc/sudoers.d/born2beroot; pam_faillock is deliberately NOT enabled
#  because the subject wants *sudo* limited, not the whole system).
#
#  These edits are what the subject means by "one or two modified files"
#  during the defence.
# =============================================================================
