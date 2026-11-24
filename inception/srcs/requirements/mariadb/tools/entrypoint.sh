#!/bin/bash
# ============================================================================ #
#  MariaDB entrypoint (42 Inception)                                          #
#                                                                              #
#  1. Initialize the data directory on first run.                           #
#  2. Start a temporary server to set the root password and create the      #
#     database + application user (credentials from Docker secrets).        #
#  3. Exec the real mysqld in the foreground (no sleep/tail).               #
# ============================================================================ #
set -e

DATADIR="/var/lib/mysql"
DB_NAME="${DB_NAME:-wordpress}"
DB_USER="${DB_USER:-wordpress}"
DB_PORT="${DB_PORT:-3306}"
DB_ADMIN="${DB_ADMIN:-inceptor}"

DB_PASSWORD="$(cat /run/secrets/db_password)"
DB_ROOT_PASSWORD="$(cat /run/secrets/db_root_password)"
DB_ADMIN_PASSWORD="$(cat /run/secrets/db_admin_password)"

# --- First run: initialize the data directory ----------------------------------
if [ ! -d "$DATADIR/mysql" ]; then
    echo ">> Initializing MariaDB data directory..."
    mariadb-install-db --user=mysql --datadir="$DATADIR" >/dev/null
fi

# --- Bootstrap server: set credentials and create database/users ------------
echo ">> Configuring MariaDB (database + users)..."
mysqld --user=mysql --datadir="$DATADIR" --skip-networking=0 \
    --bind-address=127.0.0.1 --port=0 &
TEMP_PID=$!

for i in $(seq 1 60); do
    if mysqladmin --protocol=socket ping >/dev/null 2>&1; then
        break
    fi
    if [ "$i" -eq 60 ]; then
        echo ">> ERROR: temporary MariaDB server did not start." >&2
        kill "$TEMP_PID" 2>/dev/null
        exit 1
    fi
    sleep 1
done

mysql --protocol=socket <<SQL
-- Root password for local admin access
ALTER USER 'root'@'localhost' IDENTIFIED BY '${DB_ROOT_PASSWORD}';
CREATE USER IF NOT EXISTS 'root'@'127.0.0.1' IDENTIFIED BY '${DB_ROOT_PASSWORD}';
GRANT ALL PRIVILEGES ON *.* TO 'root'@'127.0.0.1' WITH GRANT OPTION;

-- Application database + user
CREATE DATABASE IF NOT EXISTS \`${DB_NAME}\` CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
CREATE USER IF NOT EXISTS '${DB_USER}'@'%' IDENTIFIED BY '${DB_PASSWORD}';
GRANT ALL PRIVILEGES ON \`${DB_NAME}\`.* TO '${DB_USER}'@'%';
FLUSH PRIVILEGES;
SQL

kill "$TEMP_PID" 2>/dev/null
wait "$TEMP_PID" 2>/dev/null || true

echo ">> MariaDB configured. Starting production server on 0.0.0.0:${DB_PORT}..."
exec mysqld --user=mysql --datadir="$DATADIR" --bind-address=0.0.0.0 --port="$DB_PORT"
