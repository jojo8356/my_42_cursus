# ============================================================================ #
#  Tests — MariaDB configuration & entrypoint                                 #
# ============================================================================ #
from lib import check, expect, read_file


def _cnf():
    return read_file("srcs/requirements/mariadb/conf/my.cnf")


def _entry():
    return read_file("srcs/requirements/mariadb/tools/entrypoint.sh")


def test_mariadb_bind_all_interfaces():
    """MariaDB must listen on 0.0.0.0 so WordPress can reach it."""
    conf = _cnf()
    check("bind-address" in conf, "my.cnf définit bind-address")
    check("0.0.0.0" in conf, "my.cnf: bind-address = 0.0.0.0")


def test_mariadb_port_3306():
    conf = _cnf()
    check("port" in conf and "3306" in conf, "my.cnf: port 3306")


def test_mariadb_utf8mb4():
    conf = _cnf()
    check("utf8mb4" in conf, "my.cnf: character-set utf8mb4")


def test_entrypoint_initializes_datadir():
    """The entrypoint must initialize the data directory on first run."""
    entry = _entry()
    check("mariadb-install-db" in entry or "mysql_install_db" in entry,
          "entrypoint initialise le datadir")


def test_entrypoint_sets_credentials_from_secrets():
    """The entrypoint must set passwords and create DB/user from secrets."""
    entry = _entry()
    check("run/secrets/db_root_password" in entry, "entrypoint lit root password secret")
    check("run/secrets/db_password" in entry, "entrypoint lit db password secret")
    check("CREATE DATABASE" in entry, "entrypoint crée la base de données")
    check("CREATE USER" in entry, "entrypoint crée l'utilisateur DB")
    check("GRANT ALL PRIVILEGES" in entry, "entrypoint accorde les privilèges")


def test_entrypoint_execs_mysqld():
    """The entrypoint must exec the real mysqld (no sleep/tail)."""
    entry = _entry()
    check("exec mysqld" in entry, "entrypoint exec mysqld (foreground)")
    expect("tail -f" not in entry, "pas de tail -f dans l'entrypoint mariadb")
