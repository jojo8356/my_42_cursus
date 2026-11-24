# ============================================================================ #
#  Tests — WordPress + php-fpm configuration                                 #
# ============================================================================ #
from lib import check, expect, read_file


def _phpfpm():
    return read_file("srcs/requirements/wordpress/conf/php-fpm.conf")


def _entry():
    return read_file("srcs/requirements/wordpress/tools/entrypoint.sh")


def test_phpfpm_listens_tcp_9000():
    """php-fpm must listen on TCP 0.0.0.0:9000 (inter-container), not a
    Unix socket."""
    conf = _phpfpm()
    check("listen = 0.0.0.0:9000" in conf, "php-fpm listen 0.0.0.0:9000 (TCP)")
    expect("/run/php" not in conf or "0.0.0.0:9000" in conf,
           "pas d'écoute uniquement sur socket Unix", conf)
    expect("listen = /run/php" not in conf,
           "pas d'écoute sur socket Unix uniquement")


def test_phpfpm_daemonize_no():
    """php-fpm must run in foreground (daemonize = no) so docker keeps it
    alive."""
    conf = _phpfpm()
    check("daemonize = no" in conf, "php-fpm daemonize = no (foreground)")


def test_entrypoint_waits_for_mariadb():
    """The entrypoint must wait for MariaDB before installing."""
    entry = _entry()
    check("Waiting for MariaDB" in entry or "mariadb" in entry.lower(),
          "entrypoint attend MariaDB")
    check("mysql" in entry, "entrypoint vérifie la disponibilité de MariaDB")


def test_entrypoint_downloads_wordpress():
    """The entrypoint must download WordPress (no prebuilt image)."""
    entry = _entry()
    check("wordpress.org" in entry, "entrypoint télécharge WordPress (wordpress.org)")


def test_entrypoint_installs_two_users():
    """The entrypoint must install WP and create 2 users."""
    entry = _entry()
    check("wp core install" in entry, "entrypoint installe WordPress (wp core install)")
    check("wp user create" in entry, "entrypoint crée un second utilisateur (wp user create)")
    # admin + regular user both referenced
    check("WP_ADMIN" in entry and "WP_USER" in entry,
          "entrypoint utilise WP_ADMIN et WP_USER")


def test_entrypoint_admin_not_admin():
    """The admin user must not be 'admin'/'administrator'."""
    entry = _entry()
    # The actual admin username comes from env WP_ADMIN; the .env default
    # must not be admin/administrator.
    env = read_file("srcs/.env")
    for line in env.splitlines():
        if line.startswith("WP_ADMIN="):
            admin = line.split("=", 1)[1].strip()
            check(admin.lower() not in ("admin", "administrator"),
                  "WP_ADMIN n'est pas 'admin'/'administrator'", admin)
    expect("--admin_user=\"$WP_ADMIN\"" in entry,
           "installation utilise --admin_user=WP_ADMIN (pas de 'admin' en dur)",
           entry[:100])


def test_entrypoint_uses_secrets_not_env_passwords():
    """Passwords must be read from Docker secrets, not environment."""
    entry = _entry()
    check("run/secrets/wp_admin_password" in entry,
          "entrypoint lit le mot de passe admin depuis un secret")
    check("run/secrets/wp_user_password" in entry,
          "entrypoint lit le mot de passe utilisateur depuis un secret")
    check("run/secrets/db_password" in entry,
          "entrypoint lit le mot de passe DB depuis un secret")
    expect("WP_ADMIN_PASSWORD=" not in entry.replace("WP_ADMIN_PASSWORD=\"$(cat", ""),
           "password admin pas en variable env brute")


def test_entrypoint_execs_phpfpm():
    """The entrypoint must exec php-fpm in the foreground."""
    entry = _entry()
    check("exec php-fpm" in entry, "entrypoint exec php-fpm -F (foreground)")
