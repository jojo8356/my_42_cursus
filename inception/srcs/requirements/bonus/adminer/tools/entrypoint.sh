#!/bin/sh
# ============================================================================ #
#  Adminer entrypoint (BONUS) — serves the database admin UI via PHP's       #
#  built-in server on 0.0.0.0:8080, pointing at the MariaDB service.        #
# ============================================================================ #
set -e

ADMINER_DB_HOST="${ADMINER_DB_HOST:-mariadb}"
ADMINER_DB_NAME="${ADMINER_DB_NAME:-wordpress}"
ADMINER_DB_USER="${ADMINER_DB_USER:-wordpress}"

echo ">> Starting Adminer on 0.0.0.0:8080 (host: ${ADMINER_DB_HOST})..."
exec php -S 0.0.0.0:8080 /usr/local/bin/adminer.phar
