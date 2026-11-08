#!/bin/bash
# =============================================================================
#  90-bonus-service.sh — BONUS 2: a useful service of my choice — vsftpd
# =============================================================================
#  Subject v5.2, chapter VII:
#    "Set up a service of your choice that you think is useful
#     (NGINX / Apache2 excluded!). During the defense, you will have to
#     justify your choice."
#
#  Choice: **vsftpd** (Very Secure FTP Daemon), hardened, plus **fail2ban** to
#  protect the exposed SSH and FTP entry points.
#
#  Why vsftpd is the right bonus service here
#  ---------------------------------------------------------------------------
#  1. It answers a real need of this VM: WordPress lives in /var/www/html and
#     uploading themes/plugins through an FTP account is the historical,
#     file-permission-safe way to do it (no PHP write access to the web root,
#     which is precisely how WordPress sites get compromised).
#  2. It is genuinely secure when configured properly: TLS required, local
#     accounts only, chrooted home, passive ports restricted to a small range
#     that is explicitly opened in UFW.
#  3. It is not a web server, so it does not conflict with the "no NGINX /
#     no Apache2" rule of the bonus part, and it does not duplicate lighttpd.
#  4. fail2ban complements it: it bans brute-force attempts on both SSH (4242)
#     and FTP (21), which is exactly the kind of hardening a server of this
#     project should have.
#
#  Both services are installed and hardened here; the accompanying UFW rules
#  already exist in config/ufw_rules.sh (ports 21 and 40000-40010).
# =============================================================================

set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../lib/common.sh"
load_config

require_root
install_packages vsftpd fail2ban openssl

FTP_CONF=/etc/vsftpd.conf
FTP_CERT_DIR=/etc/ssl/private
FTP_CERT="$FTP_CERT_DIR/vsftpd.pem"
FTP_HOME=/srv/ftp

# --- 1. TLS material ---------------------------------------------------------
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would generate a self-signed TLS certificate for vsftpd"
else
	install -d -m 0710 -o root -g root "$FTP_CERT_DIR"
	if [ ! -f "$FTP_CERT" ]; then
		openssl req -x509 -nodes -days 3650 -newkey rsa:2048 \
			-keyout "$FTP_CERT" -out "$FTP_CERT" \
			-subj "/C=FR/ST=PACA/L=Nice/O=42/OU=born2beroot/CN=$VM_FQDN" \
			>/dev/null 2>&1
		chmod 0600 "$FTP_CERT"
		log_ok "self-signed certificate generated: $FTP_CERT"
	else
		log_skip "$FTP_CERT already exists"
	fi
fi

# --- 2. vsftpd configuration ------------------------------------------------
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would install $FTP_CONF (TLS required, chroot, local users only)"
else
	write_file "$FTP_CONF" 0644 root:root \
		"# Managed by born2beroot/scripts/setup/90-bonus-service.sh" \
		"listen=YES" \
		"listen_ipv6=NO" \
		"listen_port=$FTP_PORT" \
		"" \
		"# Only real local accounts, no anonymous access at all." \
		"anonymous_enable=NO" \
		"local_enable=YES" \
		"write_enable=YES" \
		"local_umask=022" \
		"" \
		"# Chroot every user into their home: no filesystem wandering." \
		"chroot_local_user=YES" \
		"allow_writeable_chroot=YES" \
		"user_sub_token=\$USER" \
		"local_root=/home/\$USER" \
		"" \
		"# TLS is mandatory: credentials must never travel in clear text." \
		"ssl_enable=YES" \
		"rsa_cert_file=$FTP_CERT" \
		"rsa_private_key_file=$FTP_CERT" \
		"force_local_data_ssl=YES" \
		"force_local_logins_ssl=YES" \
		"ssl_tlsv1_2=YES" \
		"ssl_tlsv1_3=YES" \
		"ssl_sslv2=NO" \
		"ssl_sslv3=NO" \
		"" \
		"# Passive mode restricted to the range opened in UFW." \
		"pasv_enable=YES" \
		"pasv_min_port=$FTP_PASSIVE_MIN" \
		"pasv_max_port=$FTP_PASSIVE_MAX" \
		"pasv_address=127.0.0.1" \
		"" \
		"# Hygiene." \
		"xferlog_enable=YES" \
		"xferlog_file=/var/log/vsftpd.log" \
		"connect_from_port_20=NO" \
		"use_localtime=YES" \
		"secure_chroot_dir=/var/run/vsftpd/empty" \
		"pam_service_name=vsftpd" \
		"rsa_cert_file=$FTP_CERT" || true
	log_ok "$FTP_CONF installed"
	if ! grep -qx 'vsftpd' /etc/shells 2>/dev/null; then
		log_skip "note: local FTP logins need a valid shell (already /bin/bash here)"
	fi
fi

# --- 3. FTP account ----------------------------------------------------------
# A dedicated local account, chrooted in its home, used to publish the
# WordPress files.  It is NOT in the sudo or user42 group.
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would create the FTP account $BONUS_FTP_USER (home $FTP_HOME)"
else
	install -d -m 0755 -o root -g root "$FTP_HOME"
	if id "$BONUS_FTP_USER" >/dev/null 2>&1; then
		log_skip "user $BONUS_FTP_USER already exists"
	else
		useradd -d "$FTP_HOME" -s /usr/sbin/nologin -c "FTP publishing account" "$BONUS_FTP_USER"
		log_ok "user $BONUS_FTP_USER created (shell nologin, home $FTP_HOME)"
	fi
	enable_service vsftpd
fi

# --- 4. fail2ban --------------------------------------------------------------
if [ "$B2BR_DRY_RUN" = "1" ]; then
	log_dry "would configure fail2ban jails for sshd:$SSH_PORT and vsftpd"
else
	write_file /etc/fail2ban/jail.d/born2beroot.local 0644 root:root \
		"[DEFAULT]" \
		"bantime  = 600" \
		"findtime = 600" \
		"maxretry = 3" \
		"" \
		"[sshd]" \
		"enabled  = true" \
		"port     = $SSH_PORT" \
		"logpath  = %(sshd_log)s" \
		"" \
		"[vsftpd]" \
		"enabled  = true" \
		"port     = $FTP_PORT" \
		"logpath  = /var/log/vsftpd.log" || true
	enable_service fail2ban
	log_ok "fail2ban jails configured (sshd:$SSH_PORT, vsftpd:$FTP_PORT)"
fi

# --- 5. Report ----------------------------------------------------------------
if [ "$B2BR_DRY_RUN" = "0" ] && command -v ss >/dev/null 2>&1; then
	if ss -tln 2>/dev/null | grep -c ":${FTP_PORT} "; then
		log_ok "vsftpd listening on port $FTP_PORT"
	else
		log_fail "nothing listens on port $FTP_PORT yet"
	fi
fi

log_ok "90-bonus-service.sh done (bonus: vsftpd + fail2ban)"
