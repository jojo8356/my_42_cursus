# ============================================================================ #
#  Tests — Bonus services (Adminer, Redis, FTP, website)                     #
# ============================================================================ #
import yaml
from lib import check, expect, read_file, path_exists


def _compose():
    return yaml.safe_load(read_file("srcs/docker-compose.yml"))


def test_all_bonus_services_defined():
    """The 4 bonus services must be defined in compose."""
    services = _compose()["services"]
    for svc in ("adminer", "redis", "ftp", "website"):
        check(svc in services, f"bonus service '{svc}' défini")


def test_bonus_services_use_profile():
    """Bonus services must be opt-in via the 'bonus' profile."""
    services = _compose()["services"]
    for svc in ("adminer", "redis", "ftp", "website"):
        profiles = services[svc].get("profiles", [])
        check("bonus" in profiles, f"bonus '{svc}' activé via profil 'bonus'")


def test_bonus_services_built_not_prebuilt():
    """Bonus services must build from local Dockerfiles."""
    services = _compose()["services"]
    for svc in ("adminer", "redis", "ftp", "website"):
        check("build" in services[svc], f"bonus '{svc}' construit via Dockerfile")
        expect("image" not in services[svc],
               f"bonus '{svc}' pas d'image pré-faite")


def test_bonus_services_have_dockerfiles():
    """Each bonus service must have its own Dockerfile + config."""
    for rel in (
        "srcs/requirements/bonus/adminer/Dockerfile",
        "srcs/requirements/bonus/redis/Dockerfile",
        "srcs/requirements/bonus/ftp/Dockerfile",
        "srcs/requirements/bonus/website/Dockerfile",
    ):
        check(path_exists(rel), f"Dockerfile bonus présent: {rel}")


def test_bonus_services_restart_policy():
    """Bonus services must also restart on crash."""
    services = _compose()["services"]
    for svc in ("adminer", "redis", "ftp", "website"):
        check(services[svc].get("restart") == "unless-stopped",
              f"bonus '{svc}' restart: unless-stopped")


def test_bonus_services_on_network():
    """Bonus services must be on the dedicated network."""
    services = _compose()["services"]
    for svc in ("adminer", "redis", "ftp", "website"):
        check("networks" in services[svc], f"bonus '{svc}' sur le réseau dédié")


def test_adminer_downloaded_not_prebuilt():
    """Adminer must be downloaded (single-file), not a prebuilt image."""
    txt = read_file("srcs/requirements/bonus/adminer/Dockerfile")
    check("adminer.org" in txt, "Adminer téléchargé depuis adminer.org")


def test_redis_binds_all_interfaces():
    """Redis must bind 0.0.0.0 for inter-container access."""
    txt = read_file("srcs/requirements/bonus/redis/Dockerfile")
    check("bind 0.0.0.0" in txt, "Redis bind 0.0.0.0")


def test_ftp_uses_vsftpd():
    """The FTP bonus must use vsftpd with a proper config."""
    dockerfile = read_file("srcs/requirements/bonus/ftp/Dockerfile")
    check("vsftpd" in dockerfile, "FTP: installe vsftpd")
    conf = read_file("srcs/requirements/bonus/ftp/conf/vsftpd.conf")
    check("listen                 = YES" in conf, "vsftpd listen = YES")
    check("chroot_local_user      = YES" in conf, "vsftpd chroot activé")


def test_ftp_entrypoint_creates_user():
    """The FTP entrypoint must create the FTP user."""
    entry = read_file("srcs/requirements/bonus/ftp/tools/entrypoint.sh")
    check("useradd" in entry, "entrypoint FTP crée l'utilisateur")
    check("vsftpd /etc/vsftpd.conf" in entry, "entrypoint lance vsftpd")


def test_website_serves_static_html():
    """The website bonus must serve a static HTML page via nginx."""
    dockerfile = read_file("srcs/requirements/bonus/website/Dockerfile")
    check("nginx" in dockerfile, "website: installe nginx")
    check("daemon off" in dockerfile, "website: nginx en foreground")
    check(path_exists("srcs/requirements/bonus/website/html/index.html"),
          "website: fichier index.html présent")
