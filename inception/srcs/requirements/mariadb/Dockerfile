# ============================================================================ #
#  MariaDB (42 Inception)                                                     #
#  Base: Debian penultimate stable (bookworm), versioned tag (no "latest").  #
#  Database server only — no nginx inside this image.                        #
# ============================================================================ #
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        mariadb-server \
        mariadb-client \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY conf/my.cnf /etc/mysql/mariadb.conf.d/50-inception.cnf
COPY tools/entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh

EXPOSE 3306
VOLUME /var/lib/mysql

ENTRYPOINT ["/entrypoint.sh"]
