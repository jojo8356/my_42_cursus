# ============================================================================ #
#  Redis (BONUS) — in-memory cache / session store                            #
#  Base: Debian penultimate stable (bookworm), versioned tag (no "latest").  #
# ============================================================================ #
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        redis-server \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Bind to all interfaces so the WordPress container can reach it over the
# Docker bridge network.
RUN echo "bind 0.0.0.0" >> /etc/redis/redis.conf

EXPOSE 6379

ENTRYPOINT ["redis-server", "/etc/redis/redis.conf"]
