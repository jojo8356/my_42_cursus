#!/bin/bash
# ============================================================================ #
#  WordPress + php-fpm entrypoint (42 Inception)                              #
#                                                                              #
#  1. Wait for MariaDB to accept connections.                               #
#  2. Download and configure WordPress (from wordpress.org, no prebuilt      #
#     DockerHub image).                                                      #
#  3. Install WordPress with 2 users — admin (WITHOUT "admin"/"administrator" #
#     in the name/username) + a regular user.                               #
#  4. Exec php-fpm in the foreground (real process, no sleep/tail).         #
# ============================================================================ #
set -e

WP_PATH="${WP_PATH:-/var/www/html}"
WP_TITLE="${WP_TITLE:-Inception}"
WP_ADMIN="${WP_ADMIN:-inceptor}"
WP_ADMIN_EMAIL="${WP_ADMIN_EMAIL:-inceptor@inception.42.fr}"
WP_USER="${WP_USER:-contributor}"
WP_USER_EMAIL="${WP_USER_EMAIL:-contributor@inception.42.fr}"
DB_HOST="${DB_HOST:-mariadb}"
DB_NAME="${DB_NAME:-wordpress}"
DB_USER="${DB_USER:-wordpress}"
DB_PORT="${DB_PORT:-3306}"

# Credentials come from Docker secrets (mounted as files), never from env
WP_ADMIN_PASSWORD="$(cat /run/secrets/wp_admin_password)"
WP_USER_PASSWORD="$(cat /run/secrets/wp_user_password)"
DB_PASSWORD="$(cat /run/secrets/db_password)"

echo ">> Waiting for MariaDB (${DB_HOST}:${DB_PORT})..."
for i in $(seq 1 60); do
    if mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASSWORD" \
            -e "SELECT 1;" "$DB_NAME" >/dev/null 2>&1; then
        echo ">> MariaDB is ready."
        break
    fi
    if [ "$i" -eq 60 ]; then
        echo ">> ERROR: MariaDB did not become ready." >&2
        exit 1
    fi
    sleep 2
done

# --- Download WordPress fresh (not a prebuilt DockerHub image) ---------------
if [ ! -f "$WP_PATH/wp-login.php" ]; then
    echo ">> Downloading WordPress..."
    mkdir -p "$WP_PATH"
    curl -fsSL https://wordpress.org/latest.tar.gz -o /tmp/wp.tar.gz
    tar -xzf /tmp/wp.tar.gz --strip-components=1 -C "$WP_PATH"
    rm -f /tmp/wp.tar.gz
    chown -R www-data:www-data "$WP_PATH"
else
    echo ">> WordPress files already present."
fi

# --- Install WordPress (once) ------------------------------------------------
if ! wp core is-installed --path="$WP_PATH" --quiet 2>/dev/null; then
    echo ">> Installing WordPress..."
    wp core install \
        --path="$WP_PATH" \
        --url="https://${DOMAIN_NAME:-inception.42.fr}" \
        --title="$WP_TITLE" \
        --admin_user="$WP_ADMIN" \
        --admin_password="$WP_ADMIN_PASSWORD" \
        --admin_email="$WP_ADMIN_EMAIL" \
        --skip-email

    echo ">> Creating regular WordPress user (${WP_USER})..."
    wp user create \
        --path="$WP_PATH" \
        --user_name="$WP_USER" \
        --user_email="$WP_USER_EMAIL" \
        --user_pass="$WP_USER_PASSWORD" \
        --role=editor \
        --allow-extra

    echo ">> WordPress installed with 2 users (admin: ${WP_ADMIN}, user: ${WP_USER})."
else
    echo ">> WordPress already installed."
fi

# --- Launch php-fpm in the foreground ----------------------------------------
echo ">> Starting php-fpm on 0.0.0.0:${WP_PORT:-9000}..."
exec php-fpm -F
