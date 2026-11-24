# ============================================================================ #
#  Tests — Dockerfiles: base images, structure, runtime commands            #
# ============================================================================ #
import os
import re
from lib import check, expect, SRC_DIR, PROJECT_ROOT, read_file


def _find_dockerfiles():
    found = []
    base = os.path.join(SRC_DIR, "requirements")
    for root, _, files in os.walk(base):
        for f in files:
            if f == "Dockerfile":
                full = os.path.join(root, f)
                found.append(os.path.relpath(full, PROJECT_ROOT))
    return found


def _dockerfile_lines(rel):
    return read_file(rel).splitlines()


def test_all_services_have_dockerfile():
    """Every service defined in compose must have a Dockerfile."""
    import yaml
    data = yaml.safe_load(read_file("srcs/docker-compose.yml"))
    dockerfiles = _find_dockerfiles()
    for name in data["services"]:
        expect(any(df in name.lower() or name in df for df in dockerfiles),
               f"service '{name}' a un Dockerfile",
               f"aucun Dockerfile trouvé pour '{name}'")


def test_base_images_debian_stable_no_latest():
    """All base images must be Debian/Alpine penultimate stable, versioned,
    never 'latest'."""
    dockerfiles = _find_dockerfiles()
    check(len(dockerfiles) >= 4, "plusieurs Dockerfiles présents")
    for rel in dockerfiles:
        froms = [l.strip() for l in _dockerfile_lines(rel)
                 if l.strip().upper().startswith("FROM")]
        check(len(froms) >= 1, f"{rel}: contient une instruction FROM")
        for f in froms:
            # Extract the image reference after "FROM"
            img = f.split(None, 1)[1].strip() if len(f.split(None, 1)) > 1 else ""
            check(img.lower().startswith(("debian:", "alpine:")),
                  f"{rel}: base Debian/Alpine", img)
            check(":latest" not in img and not img.rstrip().endswith("latest"),
                  f"{rel}: base versionée (pas 'latest')", img)
            # penultimate stable: bookworm / bullseye / alpine 3.x
            check(any(tag in img.lower() for tag in
                      ("bookworm", "bullseye", "alpine")),
                  f"{rel}: base stable connue (bookworm/bullseye/alpine)", img)


def test_no_prebuilt_dockerhub_in_dockerfiles():
    """Dockerfiles must not pull a full prebuilt app image from DockerHub
    (they install packages themselves)."""
    for rel in _find_dockerfiles():
        txt = read_file(rel)
        expect("FROM wordpress" not in txt and "FROM nginx" not in txt,
               f"{rel}: pas d'image Wordpress/NGINX pré-faite",
               "image pré-faite détectée")


def test_dockerfile_has_run_install():
    """Each Dockerfile must install its packages via RUN apt-get/apk."""
    for rel in _find_dockerfiles():
        txt = read_file(rel)
        check(("apt-get install" in txt) or ("apk add" in txt),
              f"{rel}: installe des packages via apt-get/apk")


def test_expose_and_runtime_command():
    """Each service Dockerfile must EXPOSE its port and define a real
    ENTRYPOINT/CMD (no infinite sleep/tail as the main command)."""
    for rel in _find_dockerfiles():
        txt = read_file(rel)
        check("EXPOSE" in txt, f"{rel}: déclare EXPOSE")
        has_cmd = any(k in txt for k in ("ENTRYPOINT", "CMD"))
        check(has_cmd, f"{rel}: définit ENTRYPOINT ou CMD")
        for line in txt.splitlines():
            ls = line.strip()
            if ls.upper().startswith(("ENTRYPOINT", "CMD")):
                expect("sleep" not in ls and "tail -f" not in ls,
                       f"{rel}: commande de runtime réelle (pas sleep/tail)", ls)


def test_no_infinite_sleep_tail_in_entrypoints():
    """Entrypoint scripts must exec a real process, never loop with
    sleep/tail."""
    base = os.path.join(SRC_DIR, "requirements")
    entrypoints = []
    for root, _, files in os.walk(base):
        for f in files:
            if f == "entrypoint.sh":
                entrypoints.append(os.path.relpath(os.path.join(root, f), PROJECT_ROOT))
    check(len(entrypoints) >= 3, "les entrypoints existent")
    for rel in entrypoints:
        txt = read_file(rel)
        # A real runtime: either `exec <process>` (foreground) OR launching the
        # service in the background and `wait`ing on it. No infinite
        # sleep/tail loops.
        is_exec = "exec " in txt
        is_wait = re.search(r"&\s*\n.*\bwait\b", txt, re.S) or "wait $" in txt or "wait " in txt
        check(is_exec or is_wait,
              f"{rel}: lance une vraie processus (exec ou wait sur service)")
        expect("while true; do" not in txt and "while :; do" not in txt,
               f"{rel}: pas de boucle infinie", rel)
        expect("tail -f" not in txt, f"{rel}: pas de tail -f", rel)


def test_nginx_dockerfile_has_tls_tools():
    """The nginx Dockerfile must include openssl (for TLS cert generation)."""
    txt = read_file("srcs/requirements/nginx/Dockerfile")
    check("openssl" in txt, "Dockerfile nginx: installe openssl (TLS)")


def test_wordpress_dockerfile_has_php_fpm():
    """The WordPress Dockerfile must install php-fpm and extensions."""
    txt = read_file("srcs/requirements/wordpress/Dockerfile")
    check("php-fpm" in txt, "Dockerfile wordpress: installe php-fpm")
    for ext in ("php-mysql", "php-mbstring", "php-xml"):
        check(ext in txt, f"Dockerfile wordpress: extension {ext}")


def test_mariadb_dockerfile():
    """The MariaDB Dockerfile must install the mariadb server."""
    txt = read_file("srcs/requirements/mariadb/Dockerfile")
    check("mariadb-server" in txt, "Dockerfile mariadb: installe mariadb-server")
