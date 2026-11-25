#!/bin/sh
# ============================================================================ #
#  ft_services — phpMyAdmin entrypoint                                       #
#                                                                              #
#  1. Download phpMyAdmin core (no prebuilt image).                          #
#  2. Place the config (credentials from env/Secret).                       #
#  3. Start php-fpm (background) + nginx (foreground).                      #
# ============================================================================ #
set -e

PMA_PATH="${PMA_PATH:-/var/www/html}"
PMA_VERSION="${PMA_VERSION:-5.2.1}"

if [ ! -f "$PMA_PATH/index.php" ]; then
    echo ">> Downloading phpMyAdmin ${PMA_VERSION}..."
    curl -fsSL "https://files.phpmyadmin.net/phpMyAdmin/${PMA_VERSION}/phpMyAdmin-${PMA_VERSION}-all-languages.tar.gz" \
        -o /tmp/pma.tar.gz
    tar -xzf /tmp/pma.tar.gz --strip-components=1 -C "$PMA_PATH"
    rm -f /tmp/pma.tar.gz
fi

# Config already copied in at build time (config.inc.php)
echo ">> phpMyAdmin ready (credentials from env/Secret)."

# --- Start php-fpm (background) + nginx (foreground) ------------------------
echo ">> Starting php-fpm..."
php-fpm82 -D

echo ">> Starting nginx (port 5000)..."
exec nginx -g 'daemon off;'
