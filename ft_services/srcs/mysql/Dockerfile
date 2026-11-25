# ============================================================================ #
#  ft_services — MySQL/MariaDB container (42 School)                         #
#  Base: Alpine Linux. Database for WordPress. Persistent volume on         #
#  /var/lib/mysql.                                                           #
# ============================================================================ #
FROM alpine:3.20

RUN apk add --no-cache mariadb mariadb-client ca-certificates

COPY init.sql /docker-entrypoint-initdb.d/init.sql
COPY entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh \
    && mkdir -p /var/lib/mysql /run/mysqld \
    && chown -R mysql:mysql /var/lib/mysql /run/mysqld

EXPOSE 3306

ENTRYPOINT ["/entrypoint.sh"]
