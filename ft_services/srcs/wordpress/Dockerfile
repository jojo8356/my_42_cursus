# ============================================================================ #
#  ft_services — WordPress container (42 School)                             #
#  Base: Alpine Linux. nginx + php-fpm serving WordPress on port 5050.      #
#  No prebuilt WordPress image — core is downloaded at runtime via WP-CLI.  #
# ============================================================================ #
FROM alpine:3.20

# nginx + php-fpm + PHP extensions required by WordPress + tools
RUN apk add --no-cache \
        nginx \
        php82-fpm \
        php82-mysql \
        php82-mbstring \
        php82-xml \
        php82-gd \
        php82-curl \
        php82-zip \
        curl \
        ca-certificates \
        mariadb-client

# wp-cli (single-file phar) for automated WordPress installation
RUN curl -fsSL https://raw.githubusercontent.com/wp-cli/builds/gh-pages/phar/wp-cli.phar \
        -o /usr/local/bin/wp \
    && chmod +x /usr/local/bin/wp

COPY wordpress.conf /etc/nginx/nginx.conf
COPY entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh \
    && mkdir -p /var/www/html /run/php

EXPOSE 5050

ENTRYPOINT ["/entrypoint.sh"]
