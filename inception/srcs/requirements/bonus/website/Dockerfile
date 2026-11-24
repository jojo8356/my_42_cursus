# ============================================================================ #
#  Personal website (BONUS) — static site served by NGINX                    #
#  Base: Debian penultimate stable (bookworm), versioned tag (no "latest").  #
# ============================================================================ #
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        nginx \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY html/ /var/www/html/

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
