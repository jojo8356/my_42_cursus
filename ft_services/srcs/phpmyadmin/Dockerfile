# ============================================================================ #
#  ft_services — phpMyAdmin container (42 School)                            #
#  Base: Alpine Linux. nginx + php serving phpMyAdmin on port 5000.         #
#  phpMyAdmin core is downloaded at runtime (no prebuilt image).            #
# ============================================================================ #
FROM alpine:3.20

RUN apk add --no-cache \
        nginx \
        php82 \
        php82-fpm \
        php82-mysql \
        php82-mbstring \
        php82-xml \
        curl \
        ca-certificates

COPY nginx.conf /etc/nginx/nginx.conf
COPY config.inc.php /var/www/html/config.inc.php
COPY entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh \
    && mkdir -p /var/www/html /run/php

EXPOSE 5000

ENTRYPOINT ["/entrypoint.sh"]
