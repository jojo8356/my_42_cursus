# ============================================================================ #
#  Tests — NGINX configuration                                                #
# ============================================================================ #
from lib import check, expect, read_file


def _conf():
    return read_file("srcs/requirements/nginx/conf/nginx.conf")


def test_nginx_listens_443_ssl_only():
    """Nginx must listen on 443 with SSL; no plain HTTP listener."""
    conf = _conf()
    check("listen 443 ssl" in conf, "nginx listen 443 ssl")
    expect("listen 80" not in conf, "pas d'écoute HTTP brute sur le 80", conf[:120])


def test_nginx_tls_12_and_13_only():
    """TLS protocols must be restricted to 1.2 and 1.3."""
    conf = _conf()
    check("ssl_protocols" in conf, "nginx définit ssl_protocols")
    check("TLSv1.2" in conf and "TLSv1.3" in conf,
          "nginx active TLS 1.2 et TLS 1.3")
    expect("TLSv1 " not in conf and "TLSv1.1" not in conf,
           "nginx n'active pas TLS 1.0/1.1 (obsolète)")


def test_nginx_fastcgi_to_wordpress_9000():
    """Nginx must forward PHP to wordpress:9000 via FastCGI."""
    conf = _conf()
    check("fastcgi_pass" in conf, "nginx utilise fastcgi_pass")
    check("wordpress:9000" in conf, "fastcgi_pass pointe vers wordpress:9000")
    check("fastcgi_param" in conf and "SCRIPT_FILENAME" in conf,
          "nginx définit SCRIPT_FILENAME pour php")


def test_nginx_root_serves_wordpress_volume():
    """Nginx root must be the WordPress shared volume path."""
    conf = _conf()
    check("/var/www/html" in conf, "root de nginx pointe sur /var/www/html")


def test_nginx_certificate_config():
    """Nginx config must reference certificate and key paths."""
    conf = _conf()
    check("ssl_certificate" in conf, "nginx référence ssl_certificate")
    check("ssl_certificate_key" in conf, "nginx référence ssl_certificate_key")


def test_nginx_entrypoint_generates_certificate():
    """The entrypoint must generate a self-signed cert if absent."""
    entry = read_file("srcs/requirements/nginx/tools/entrypoint.sh")
    check("openssl req" in entry, "entrypoint génère un certificat (openssl)")
    check("exec nginx" in entry, "entrypoint lance nginx en foreground")
