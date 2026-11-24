# ============================================================================ #
#  Tests — phpMyAdmin container                                             #
# ============================================================================ #
from lib import check, expect, read_file


def test_nginx_listens_5000():
    conf = read_file("srcs/phpmyadmin/nginx.conf")
    check("listen      5000" in conf or "listen 5000" in conf,
          "phpmyadmin nginx: écoute le port 5000")
    check("fastcgi_pass" in conf, "phpmyadmin nginx: proxy vers php-fpm")


def test_config_inc_php_uses_env():
    cfg = read_file("srcs/phpmyadmin/config.inc.php")
    check("getenv" in cfg, "config.inc.php: lit les credentials via getenv (env/Secret)")
    check("PMA_HOST" in cfg, "config.inc.php: PMA_HOST configurable")
    check("blowfish_secret" in cfg, "config.inc.php: blowfish_secret défini")


def test_config_not_hardcoded_password():
    cfg = read_file("srcs/phpmyadmin/config.inc.php")
    # No literal password value should be assigned; auth is cookie-based and
    # credentials come from env. Detect patterns like 'password' => 'literal'.
    for line in cfg.splitlines():
        ls = line.lower()
        if "password" in ls and "=>" in ls and "'" in ls:
            # must be an env reference, not a literal
            expect("getenv" in ls or "$" in ls,
                   "config.inc.php: pas de mot de passe littéral", line.strip())


def test_entrypoint_downloads_phpmyadmin():
    entry = read_file("srcs/phpmyadmin/entrypoint.sh")
    check("phpmyadmin" in entry.lower(), "entrypoint: gère phpMyAdmin")
    check("curl" in entry or "wget" in entry, "entrypoint: télécharge phpMyAdmin")
    check("exec nginx" in entry, "entrypoint: lance nginx en foreground")


def test_dockerfile_installs_php_and_nginx():
    dockerfile = read_file("srcs/phpmyadmin/Dockerfile")
    check("php82" in dockerfile, "Dockerfile phpmyadmin: installe php")
    check("nginx" in dockerfile, "Dockerfile phpmyadmin: installe nginx")
    check("EXPOSE 5000" in dockerfile, "Dockerfile phpmyadmin: expose 5000")
