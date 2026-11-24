# ============================================================================ #
#  WordPress + php-fpm (42 Inception)                                         #
#  Base: Debian penultimate stable (bookworm), versioned tag (no "latest").  #
#  No nginx inside this image — it only serves the app over FastCGI :9000.   #
# ============================================================================ #
FROM debian:bookworm-slim

# php-fpm + PHP extensions required by WordPress (mysqli, mbstring, xml, gd, curl)
RUN apt-get update && apt-get install -y --no-install-recommends \
        php-fpm \
        php-mysql \
        php-mbstring \
        php-xml \
        php-gd \
        php-curl \
        curl \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# php-fpm must listen on a TCP port (9000) — not a Unix socket, because the
# NGINX container communicates with it over the Docker bridge network.
COPY conf/php-fpm.conf /etc/php/8.2/fpm/php-fpm.conf

# WordPress application files
COPY tools/entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh

EXPOSE 9000
VOLUME /var/www/html

ENTRYPOINT ["/entrypoint.sh"]
