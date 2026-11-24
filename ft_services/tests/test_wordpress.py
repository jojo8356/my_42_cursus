# ============================================================================ #
#  Tests — WordPress container (nginx + php-fpm, WP-CLI, 2 users)           #
# ============================================================================ #
from lib import check, expect, read_file


def _conf():
    return read_file("srcs/wordpress/wordpress.conf")


def _entry():
    return read_file("srcs/wordpress/entrypoint.sh")


def test_wordpress_listens_5050():
    conf = _conf()
    check("listen      5050" in conf or "listen 5050" in conf,
          "wordpress nginx: écoute le port 5050")


def test_wordpress_php_fpm():
    conf = _conf()
    check("fastcgi_pass" in conf, "wordpress nginx: utilise fastcgi_pass")
    check("php-fpm" in conf or "run/php" in conf,
          "wordpress nginx: proxy vers php-fpm")


def test_entrypoint_waits_for_mysql():
    entry = _entry()
    check("Waiting for MySQL" in entry or "mysql" in entry.lower(),
          "entrypoint wordpress: attend MySQL")
    check("mysql -h" in entry, "entrypoint wordpress: vérifie MySQL")


def test_entrypoint_uses_wp_cli():
    entry = _entry()
    check("wp core download" in entry, "entrypoint: télécharge WP (wp core download)")
    check("wp config create" in entry, "entrypoint: crée wp-config")
    check("wp core install" in entry, "entrypoint: installe WordPress")


def test_entrypoint_two_users_admin_not_admin():
    entry = _entry()
    check("wp core install" in entry, "entrypoint: installe WordPress")
    check("wp user create" in entry, "entrypoint: crée un 2e utilisateur")
    # admin username must not be 'admin'/'administrator'
    configmap = read_file("yaml/configmap.yaml")
    for line in configmap.splitlines():
        if line.startswith("WP_ADMIN:") or line.startswith("  WP_ADMIN:"):
            admin = line.split(":", 1)[1].strip().strip('"')
            check(admin.lower() not in ("admin", "administrator"),
                  "WP_ADMIN n'est pas 'admin'/'administrator'", admin)
            break
    check("--admin_user=" in entry, "entrypoint: utilise --admin_user")


def test_entrypoint_starts_phpfpm_and_nginx():
    entry = _entry()
    check("php-fpm" in entry, "entrypoint: lance php-fpm")
    check("exec nginx" in entry, "entrypoint: lance nginx en foreground")


def test_wordpress_dockerfile_has_php_fpm():
    dockerfile = read_file("srcs/wordpress/Dockerfile")
    check("php82-fpm" in dockerfile or "php-fpm" in dockerfile,
          "Dockerfile wordpress: installe php-fpm")
    check("nginx" in dockerfile, "Dockerfile wordpress: installe nginx")
