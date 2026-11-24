# ============================================================================ #
#  Tests — cross-cutting subject rules & bonus compliance                     #
# ============================================================================ #
import os
from lib import check, expect, SRC_DIR, PROJECT_ROOT, read_file, path_exists


def test_all_images_alpine_base():
    """Every Dockerfile must use an Alpine base (no other base / no latest)."""
    for root, _, files in os.walk(SRC_DIR):
        for f in files:
            if f == "Dockerfile":
                rel = os.path.relpath(os.path.join(root, f), PROJECT_ROOT)
                txt = read_file(rel)
                for line in txt.splitlines():
                    ls = line.strip()
                    if ls.upper().startswith("FROM"):
                        img = ls.split(None, 1)[1].strip()
                        check(img.lower().startswith("alpine:"),
                              f"{rel}: base Alpine", img)
                        check(":latest" not in img,
                              f"{rel}: pas de 'latest'", img)


def test_no_prebuilt_hub_images_anywhere():
    """No Dockerfile may pull a full prebuilt service image from a hub."""
    forbidden = ("FROM wordpress", "FROM nginx", "FROM mysql", "FROM mariadb",
                 "FROM grafana", "FROM influxdb", "FROM phpmyadmin",
                 "FROM telegraf", "FROM ssh")
    for root, _, files in os.walk(SRC_DIR):
        for f in files:
            if f == "Dockerfile":
                rel = os.path.relpath(os.path.join(root, f), PROJECT_ROOT)
                txt = read_file(rel)
                for bad in forbidden:
                    expect(bad not in txt, f"{rel}: pas d'image pré-faite '{bad}'")


def test_no_hardcoded_real_passwords():
    """Committed files must not contain real password values (only env/Secret
    references or placeholders)."""
    suspicious = ("password=\"", "password='", "passwd=", "IDENTIFIED BY '")
    for root, _, files in os.walk(SRC_DIR):
        for f in files:
            if f.endswith((".sh", ".conf", ".php", ".sql", ".cnf", ".conf")):
                rel = os.path.relpath(os.path.join(root, f), PROJECT_ROOT)
                txt = read_file(rel)
                # Allow env-var references and placeholders
                for sus in suspicious:
                    for line in txt.splitlines():
                        ls = line.strip()
                        if sus in ls:
                            # Must be an env reference (getenv/$VAR) or placeholder
                            allowed = ("getenv" in ls or "$" in ls or
                                       "__" in ls or "${DB_PASSWORD}" in ls
                                       or "IDENTIFIED BY '${" in ls
                                       or "IDENTIFIED BY \"${" in ls)
                            expect(allowed,
                                   f"{rel}: pas de mot de passe en dur", ls[:80])


def test_bonus_services_all_implemented():
    """All 4 bonus services must be present (Dockerfile + manifest)."""
    for svc in ("grafana", "influxdb", "telegraf", "ssh"):
        check(path_exists(f"srcs/bonus/{svc}/Dockerfile"),
              f"bonus '{svc}': Dockerfile présent")
        check(path_exists(f"yaml/bonus/{svc}.yaml"),
              f"bonus '{svc}': manifest présent")


def test_nginx_redirect_and_proxy_pattern():
    """nginx must both reverse-proxy phpMyAdmin AND 307-redirect to WordPress."""
    conf = read_file("srcs/nginx/nginx.conf")
    check("location /phpmyadmin" in conf, "nginx: reverse proxy phpMyAdmin")
    check("return 307" in conf, "nginx: 307 redirect")
    check("5050" in conf, "nginx: 307 redirect vers WordPress (5050)")


def test_wordpress_two_users_rule():
    """WordPress must define 2 users with admin not 'admin/administrator'."""
    entry = read_file("srcs/wordpress/entrypoint.sh")
    check("wp core install" in entry, "WordPress: installation via WP-CLI")
    check("wp user create" in entry, "WordPress: 2e utilisateur créé")
    cm = read_file("yaml/configmap.yaml")
    for line in cm.splitlines():
        if line.strip().startswith("WP_ADMIN:"):
            admin = line.split(":", 1)[1].strip().strip('"')
            check(admin.lower() not in ("admin", "administrator"),
                  "WordPress: admin != 'admin'/'administrator'", admin)


def test_setup_covers_bonus():
    """setup.sh must handle all bonus services in its image build + deploy."""
    txt = read_file("setup.sh")
    for svc in ("grafana", "influxdb", "telegraf", "ssh"):
        check(svc in txt, f"setup.sh: gère le bonus '{svc}'")


def test_persistent_volumes_for_stateful():
    """WordPress, MySQL, Grafana, InfluxDB must have persistent storage."""
    pvc = read_file("yaml/pvc.yaml")
    for claim in ("wordpress-pvc", "mysql-pvc", "grafana-pvc", "influxdb-pvc"):
        check(claim in pvc, f"PVC '{claim}' défini")


def test_subject_file_present():
    check(path_exists("subject.md"), "subject.md (énoncé) présent")
    check(path_exists("README.md"), "README.md présent")
