# ============================================================================ #
#  Tests — docker-compose.yml structure & mandatory rules                    #
# ============================================================================ #
import yaml
from lib import check, expect, SRC_DIR, read_file


def _load():
    return yaml.safe_load(read_file("srcs/docker-compose.yml"))


def test_compose_yaml_is_valid():
    """The compose file must be valid, well-formed YAML."""
    data = _load()
    check(isinstance(data, dict), "docker-compose.yml est un mapping")
    check("services" in data, "docker-compose.yml contient 'services'")


def test_mandatory_services_present():
    """NGINX + WordPress + MariaDB must all be defined."""
    services = _load()["services"]
    for svc in ("nginx", "wordpress", "mariadb"):
        check(svc in services, f"service obligatoire '{svc}' présent")


def test_images_are_built_not_prebuilt():
    """Every service must build from a local Dockerfile (no prebuilt DockerHub
    images, no remote 'image:' references)."""
    services = _load()["services"]
    for name, cfg in services.items():
        if "build" not in cfg:
            expect(False, f"service '{name}' utilise 'build' (pas d'image pré-faite)",
                   "pas de directive 'build'")
        else:
            check(True, f"service '{name}' construit depuis un Dockerfile")
        expect("image" not in cfg,
               f"service '{name}' n'utilise pas une image pré-faite de DockerHub",
               "'image:' présent")


def test_dedicated_bridge_network():
    """There must be a dedicated bridge network, no host network, no links."""
    data = _load()
    networks = data.get("networks", {})
    check(len(networks) >= 1, "un réseau dédié est défini")
    check("inception" in networks, "réseau 'inception' présent")
    for name, cfg in networks.items():
        if "driver" in cfg:
            check(cfg["driver"] == "bridge", f"réseau '{name}' est un réseau bridge")
    for name, cfg in data["services"].items():
        expect("network" not in cfg or cfg.get("network") != "host",
               f"service '{name}' n'utilise pas network: host",
               "network: host détecté")
        expect("links" not in cfg,
               f"service '{name}' n'utilise pas 'links:' (déprécié)",
               "'links:' détecté")
        check("networks" in cfg, f"service '{name}' est attaché au réseau")


def test_restart_policy():
    """Every service must restart on crash (restart: unless-stopped)."""
    for name, cfg in _load()["services"].items():
        check("restart" in cfg and cfg["restart"] == "unless-stopped",
              f"service '{name}' a restart: unless-stopped")


def test_named_volumes():
    """There must be named volumes for DB + WordPress files (no direct bind
    mounts for these two)."""
    data = _load()
    volumes = data.get("volumes", {})
    check("vol-mariadb" in volumes, "volume nommé 'vol-mariadb' présent")
    check("vol-wordpress" in volumes, "volume nommé 'vol-wordpress' présent")
    # Named volumes must be referenced via driver_opts (bind to home dir),
    # not used as raw host bind mounts in services.
    for name, cfg in data["services"].items():
        for vol in cfg.get("volumes", []):
            check(":" not in vol or ":ro" in vol or ":rw" in vol or vol.startswith("vol-"),
                  f"service '{name}' volume '{vol}' est un volume nommé ou ro/rw",
                  "bind mount brut suspect")


def test_depends_on_ordering():
    """Wordpress depends on MariaDB; Nginx depends on Wordpress."""
    services = _load()["services"]
    check("mariadb" in services["wordpress"].get("depends_on", []),
          "wordpress dépend de mariadb")
    check("wordpress" in services["nginx"].get("depends_on", []),
          "nginx dépend de wordpress")


def test_nginx_only_public_port_443():
    """Nginx must expose 443 publicly; no other public ports on mandatory svcs."""
    services = _load()["services"]
    nginx_ports = services["nginx"].get("ports", [])
    check(any("443" in str(p) for p in nginx_ports),
          "nginx expose le port 443 publiquement")
    # WordPress & MariaDB must NOT expose their ports publicly
    for svc in ("wordpress", "mariadb"):
        expect("ports" not in services[svc],
               f"service '{svc}' n'expose pas de port publiquement",
               "'ports' détecté")


def test_secrets_referenced():
    """Credentials must be passed via Docker secrets (not env passwords)."""
    data = _load()
    check("secrets" in data, "définition 'secrets:' présente")
    svc_secrets = []
    for name, cfg in data["services"].items():
        if "secrets" in cfg:
            svc_secrets.extend(cfg["secrets"])
    check(len(svc_secrets) >= 3, "au moins 3 secrets sont utilisés par les services")


def test_two_wp_users_and_admin_not_admin():
    """WordPress must define 2 users, and the admin name must NOT be
    'admin' or 'administrator'."""
    data = _load()
    wp = data["services"]["wordpress"]
    env = wp.get("environment", {})
    admin = env.get("WP_ADMIN", "")
    check(admin and admin.lower() not in ("admin", "administrator"),
          "nom d'admin WP ne contient pas 'admin'/'administrator'",
          f"WP_ADMIN={admin}")
    check("WP_USER" in env, "deuxième utilisateur WordPress (WP_USER) défini")
    check("WP_ADMIN_PASSWORD" not in env,
          "password admin WP via secret (pas en env)",
          "WP_ADMIN_PASSWORD en env")
