# ============================================================================ #
#  Tests — Dockerfiles: Alpine base, no latest, no prebuilt hub, real cmd     #
# ============================================================================ #
import os
from lib import check, expect, SRC_DIR, PROJECT_ROOT, read_file


def _find_dockerfiles():
    found = []
    for root, _, files in os.walk(SRC_DIR):
        for f in files:
            if f == "Dockerfile":
                found.append(os.path.relpath(os.path.join(root, f), PROJECT_ROOT))
    return sorted(found)


def _dockerfile_lines(rel):
    return read_file(rel).splitlines()


def test_all_services_have_dockerfile():
    """Every service (mandatory + bonus) must have a Dockerfile."""
    services = ("nginx", "ftp", "wordpress", "mysql", "phpmyadmin",
                "grafana", "influxdb", "telegraf", "ssh")
    dockerfiles = _find_dockerfiles()
    for svc in services:
        expect(any(f.endswith(f"/{svc}/Dockerfile") for f in dockerfiles),
               f"service '{svc}' a un Dockerfile",
               f"aucun Dockerfile pour '{svc}'")


def test_base_images_alpine_no_latest():
    """All base images must be Alpine, versioned, never 'latest'."""
    dockerfiles = _find_dockerfiles()
    check(len(dockerfiles) >= 9, "tous les Dockerfiles sont présents")
    for rel in dockerfiles:
        froms = [l.strip() for l in _dockerfile_lines(rel)
                 if l.strip().upper().startswith("FROM")]
        check(len(froms) >= 1, f"{rel}: contient FROM")
        for f in froms:
            img = f.split(None, 1)[1].strip() if len(f.split(None, 1)) > 1 else ""
            check(img.lower().startswith("alpine:"),
                  f"{rel}: base Alpine", img)
            check(":latest" not in img and not img.rstrip().endswith("latest"),
                  f"{rel}: base versionée (pas 'latest')", img)


def test_no_prebuilt_hub_images():
    """Dockerfiles must not pull prebuilt service images from a hub."""
    forbidden = ("FROM wordpress", "FROM nginx", "FROM mysql", "FROM mariadb",
                 "FROM grafana", "FROM influxdb", "FROM phpmyadmin", "FROM telegraf")
    for rel in _find_dockerfiles():
        txt = read_file(rel)
        for bad in forbidden:
            expect(bad not in txt, f"{rel}: pas d'image pré-faite '{bad}'")


def test_dockerfile_installs_packages():
    """Each Dockerfile must install its packages (apk add)."""
    for rel in _find_dockerfiles():
        txt = read_file(rel)
        check(("apk add" in txt) or ("apk --no-cache add" in txt),
              f"{rel}: installe des packages via apk")


def test_expose_and_real_command():
    """Each service Dockerfile must EXPOSE its port (except purely-internal
    collectors like telegraf) and define a real ENTRYPOINT/CMD."""
    for rel in _find_dockerfiles():
        txt = read_file(rel)
        # telegraf is a purely-internal metrics collector (no listening port)
        if "telegraf" not in rel:
            check("EXPOSE" in txt, f"{rel}: déclare EXPOSE")
        has_cmd = any(k in txt for k in ("ENTRYPOINT", "CMD"))
        check(has_cmd, f"{rel}: définit ENTRYPOINT ou CMD")
        for line in txt.splitlines():
            ls = line.strip()
            if ls.upper().startswith(("ENTRYPOINT", "CMD")):
                expect("sleep" not in ls and "tail -f" not in ls,
                       f"{rel}: commande réelle (pas sleep/tail)", ls)


def test_entrypoints_syntax_and_real_process():
    """Entrypoint scripts must pass bash -n and exec a real process."""
    import lib
    entrypoints = []
    for root, _, files in os.walk(SRC_DIR):
        for f in files:
            if f == "entrypoint.sh":
                entrypoints.append(os.path.relpath(os.path.join(root, f), PROJECT_ROOT))
    check(len(entrypoints) >= 4, "les entrypoints existent")
    for rel in entrypoints:
        ok, err = lib.bash_syntax_ok(rel)
        check(ok, f"{rel}: syntaxe shell valide (bash -n)", err)
        txt = read_file(rel)
        is_exec = "exec " in txt
        is_wait = "wait " in txt
        check(is_exec or is_wait,
              f"{rel}: lance un vrai processus (exec ou wait)")
        expect("tail -f" not in txt, f"{rel}: pas de tail -f", rel)
        expect("while true; do" not in txt, f"{rel}: pas de boucle infinie", rel)
