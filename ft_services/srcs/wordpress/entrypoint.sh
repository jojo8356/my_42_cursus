#!/bin/sh
# ============================================================================ #
#  ft_services — WordPress entrypoint                                        #
#                                                                              #
#  1. Wait for the MySQL container to accept connections.                   #
#  2. Download WordPress core (WP-CLI) — no prebuilt image.                #
#  3. Install WordPress + create 2 users (admin without                     #
#     'admin'/'administrator').                                              #
#  4. Start php-fpm (background) then nginx in the foreground.             #
# ============================================================================ #
set -e

WP_PATH="${WP_PATH:-/var/www/html}"
WP_URL="${WP_URL:-http://localhost:5050}"
WP_TITLE="${WP_TITLE:-ft_services}"
WP_ADMIN="${WP_ADMIN:-webmaster}"
WP_ADMIN_EMAIL="${WP_ADMIN_EMAIL:-webmaster@42.fr}"
WP_USER="${WP_USER:-editor}"
WP_USER_EMAIL="${WP_USER_EMAIL:-editor@42.fr}"
DB_HOST="${DB_HOST:-mysql}"
DB_NAME="${DB_NAME:-wordpress}"
DB_USER="${DB_USER:-wordpress}"
DB_PASSWORD="${DB_PASSWORD:-}"

# --- Wait for MySQL ----------------------------------------------------------
echo ">> Waiting for MySQL (${DB_HOST})..."
for i in $(seq 1 60); do
    if mysql -h "$DB_HOST" -u "$DB_USER" -p"$DB_PASSWORD" -e "SELECT 1;" "$DB_NAME" >/dev/null 2>&1; then
        echo ">> MySQL is ready."
        break
    fi
    if [ "$i" -eq 60 ]; then
        echo ">> ERROR: MySQL did not become ready." >&2
        exit 1
    fi
    sleep 2
done

# --- Download + install WordPress --------------------------------------------
mkdir -p "$WP_PATH"
if [ ! -f "$WP_PATH/wp-login.php" ]; then
    echo ">> Downloading WordPress..."
    wp core download --path="$WP_PATH" --allow-root --skip-content >/dev/null
fi

if ! wp core is-installed --path="$WP_PATH" --allow-root >/dev/null 2>&1; then
    echo ">> Installing WordPress..."
    wp config create \
        --path="$WP_PATH" --allow-root \
        --dbname="$DB_NAME" --dbuser="$DB_USER" \
        --dbpass="$DB_PASSWORD" --dbhost="$DB_HOST" >/dev/null

    wp core install \
        --path="$WP_PATH" --allow-root \
        --url="$WP_URL" --title="$WP_TITLE" \
        --admin_user="$WP_ADMIN" \
        --admin_password="$WP_ADMIN_PASSWORD" \
        --admin_email="$WP_ADMIN_EMAIL" --skip-email >/dev/null

    echo ">> Creating second WordPress user (${WP_USER})..."
    wp user create \
        --path="$WP_PATH" --allow-root \
        --user_name="$WP_USER" \
        --user_email="$WP_USER_EMAIL" \
        --user_pass="$WP_USER_PASSWORD" \
        --role=editor >/dev/null

    echo ">> WordPress installed with 2 users (${WP_ADMIN}, ${WP_USER})."
else
    echo ">> WordPress already installed."
fi

chown -R www-data:www-data "$WP_PATH" 2>/dev/null || chown -R nginx:nginx "$WP_PATH"

# --- Start php-fpm (background) + nginx (foreground) ------------------------
echo ">> Starting php-fpm..."
php-fpm82 -D

echo ">> Starting nginx (port 5050)..."
exec nginx -g 'daemon off;'
