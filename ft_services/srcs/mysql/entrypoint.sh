#!/bin/sh
# ============================================================================ #
#  ft_services — MySQL/MariaDB entrypoint                                   #
#                                                                              #
#  1. Initialize the data directory on first run.                          #
#  2. Start a temporary server to create the user (password from Secret),  #
#     create the database + grants (init.sql), then flush.                  #
#  3. Exec the real mysqld in the foreground (no sleep/tail).              #
# ============================================================================ #
set -e

DATADIR="/var/lib/mysql"
DB_NAME="${DB_NAME:-wordpress}"
DB_USER="${DB_USER:-wordpress}"
DB_PASSWORD="${DB_PASSWORD:-}"

# --- First run: initialize the data directory ---------------------------------
if [ ! -d "$DATADIR/mysql" ]; then
    echo ">> Initializing MariaDB data directory..."
    mariadb-install-db --user=mysql --datadir="$DATADIR" >/dev/null
fi

# --- Bootstrap server: create user + database + grants ------------------------
echo ">> Configuring MariaDB (user, database, grants)..."
mysqld --user=mysql --datadir="$DATADIR" --skip-networking --bind-address=127.0.0.1 &
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

# Create the application user with the password from the Secret
mysql --protocol=socket <<SQL
CREATE USER IF NOT EXISTS '${DB_USER}'@'%' IDENTIFIED BY '${DB_PASSWORD}';
SQL

# Create database + grants from init.sql
mysql --protocol=socket "$DB_NAME" < /docker-entrypoint-initdb.d/init.sql
mysql --protocol=socket <<SQL
GRANT ALL PRIVILEGES ON \`${DB_NAME}\`.* TO '${DB_USER}'@'%';
FLUSH PRIVILEGES;
SQL

kill "$TEMP_PID" 2>/dev/null
wait "$TEMP_PID" 2>/dev/null || true

echo ">> MariaDB configured. Starting production server on 0.0.0.0:3306..."
exec mysqld --user=mysql --datadir="$DATADIR" --bind-address=0.0.0.0 --port=3306
