# ============================================================================ #
#  Tests — NGINX configuration (SSL, reverse proxy, 307 redirect)            #
# ============================================================================ #
from lib import check, expect, read_file


def _conf():
    return read_file("srcs/nginx/nginx.conf")


def test_nginx_listens_80_and_443():
    conf = _conf()
    check("listen      80" in conf or "listen 80" in conf, "nginx écoute le 80 (HTTP)")
    check("listen      443 ssl" in conf or "listen 443 ssl" in conf,
          "nginx écoute le 443 ssl")


def test_nginx_tls_12_13():
    conf = _conf()
    check("ssl_protocols" in conf, "nginx définit ssl_protocols")
    check("TLSv1.2" in conf and "TLSv1.3" in conf, "nginx: TLS 1.2 et 1.3")
    expect("TLSv1.1" not in conf, "nginx: pas TLS 1.1 (obsolète)")


def test_nginx_http_to_https_redirect():
    conf = _conf()
    check("return 307 https://" in conf, "nginx: redirige HTTP vers HTTPS (307)")


def test_nginx_reverse_proxy_phpmyadmin():
    conf = _conf()
    check("location /phpmyadmin" in conf, "nginx: reverse proxy /phpmyadmin")
    check("proxy_pass" in conf, "nginx: utilise proxy_pass")
    check("phpmyadmin:5000" in conf, "nginx: proxy vers phpmyadmin:5000")


def test_nginx_307_redirect_to_wordpress():
    conf = _conf()
    check("return 307" in conf, "nginx: utilise un 307")
    # 307 redirect target is wordpress on 5050
    check("5050" in conf, "nginx: 307 redirect vers le port 5050 (WordPress)")


def test_nginx_certificate_config():
    conf = _conf()
    check("ssl_certificate" in conf, "nginx: référence ssl_certificate")
    check("ssl_certificate_key" in conf, "nginx: référence ssl_certificate_key")


def test_nginx_entrypoint_generates_cert():
    entry = read_file("srcs/nginx/entrypoint.sh")
    check("openssl req" in entry, "entrypoint nginx: génère un certificat (openssl)")
    check("exec nginx" in entry, "entrypoint nginx: lance nginx en foreground")
