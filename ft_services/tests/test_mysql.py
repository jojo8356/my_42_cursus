# ============================================================================ #
#  Tests — MySQL/MariaDB container                                           #
# ============================================================================ #
from lib import check, expect, read_file


def test_init_sql_creates_database():
    sql = read_file("srcs/mysql/init.sql")
    check("CREATE DATABASE" in sql, "init.sql: crée la base de données")
    check("wordpress" in sql, "init.sql: base 'wordpress'")


def test_init_sql_grants():
    sql = read_file("srcs/mysql/init.sql")
    check("GRANT" in sql, "init.sql: accorde des privilèges")
    check("FLUSH PRIVILEGES" in sql, "init.sql: flush des privilèges")


def test_entrypoint_initializes_datadir():
    entry = read_file("srcs/mysql/entrypoint.sh")
    check("mariadb-install-db" in entry,
          "entrypoint: initialise le datadir (mariadb-install-db)")


def test_entrypoint_creates_user_with_password():
    entry = read_file("srcs/mysql/entrypoint.sh")
    check("CREATE USER" in entry, "entrypoint: crée l'utilisateur DB")
    check("IDENTIFIED BY" in entry, "entrypoint: mot de passe via Secret")
    check("DB_PASSWORD" in entry, "entrypoint: utilise DB_PASSWORD (secret)")


def test_entrypoint_runs_init_sql():
    entry = read_file("srcs/mysql/entrypoint.sh")
    check("init.sql" in entry, "entrypoint: exécute init.sql")


def test_entrypoint_execs_mysqld():
    entry = read_file("srcs/mysql/entrypoint.sh")
    check("exec mysqld" in entry, "entrypoint: exec mysqld (foreground)")
    expect("tail -f" not in entry, "entrypoint: pas de tail -f")


def test_dockerfile_installs_mariadb():
    dockerfile = read_file("srcs/mysql/Dockerfile")
    check("mariadb" in dockerfile, "Dockerfile mysql: installe mariadb")
    check("EXPOSE 3306" in dockerfile, "Dockerfile mysql: expose 3306")
