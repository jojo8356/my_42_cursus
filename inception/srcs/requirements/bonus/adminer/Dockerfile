# ============================================================================ #
#  Adminer (BONUS) — database administration interface                     #
#  Base: Debian penultimate stable (bookworm), versioned tag (no "latest").  #
# ============================================================================ #
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        php-cli \
        php-mysql \
        curl \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Download Adminer (single-file PHP app) from its official site — no
# prebuilt DockerHub image used.
RUN curl -fsSL https://www.adminer.org/adminer.phar -o /usr/local/bin/adminer.phar \
    && chmod +x /usr/local/bin/adminer.phar

COPY tools/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8080

ENTRYPOINT ["/entrypoint.sh"]
