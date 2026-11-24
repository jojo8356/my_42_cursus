# ============================================================================ #
#  NGINX — TLS-only web server / reverse proxy (42 Inception)                 #
#  Base: Debian penultimate stable (bookworm), versioned tag (no "latest").  #
# ============================================================================ #
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        nginx \
        openssl \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY conf/nginx.conf /etc/nginx/nginx.conf
COPY tools/entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh

EXPOSE 443
VOLUME /var/www/html

ENTRYPOINT ["/entrypoint.sh"]
