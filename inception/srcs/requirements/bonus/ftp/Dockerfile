# ============================================================================ #
#  FTP server (BONUS) — vsftpd                                               #
#  Base: Debian penultimate stable (bookworm), versioned tag (no "latest").  #
# ============================================================================ #
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        vsftpd \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY conf/vsftpd.conf /etc/vsftpd.conf
COPY tools/entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh

EXPOSE 21
VOLUME /srv/ftp

ENTRYPOINT ["/entrypoint.sh"]
