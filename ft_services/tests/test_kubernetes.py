# ============================================================================ #
#  Tests — Kubernetes manifests (YAML validity, services, ports, PVC, probes) #
# ============================================================================ #
import os
import yaml
from lib import check, expect, YAML_DIR, PROJECT_ROOT, read_file, path_exists


def _yaml_docs(rel):
    """Parse a (possibly multi-document) YAML file into a list of dicts."""
    text = read_file(rel)
    docs = []
    for chunk in text.split("\n---"):
        chunk = chunk.strip()
        if not chunk:
            continue
        docs.append(yaml.safe_load(chunk))
    return [d for d in docs if d]


def _all_manifests():
    found = []
    for root, _, files in os.walk(YAML_DIR):
        for f in files:
            if f.endswith(".yaml"):
                found.append(os.path.relpath(os.path.join(root, f), PROJECT_ROOT))
    return sorted(found)


def test_all_manifests_valid_yaml():
    for rel in _all_manifests():
        try:
            _yaml_docs(rel)
            check(True, f"{rel}: YAML valide")
        except Exception as exc:
            expect(False, f"{rel}: YAML valide", str(exc))


def test_mandatory_services_have_manifests():
    for svc in ("nginx", "ftp", "wordpress", "mysql", "phpmyadmin"):
        expect(path_exists(f"yaml/{svc}.yaml"),
               f"manifest '{svc}.yaml' présent")


def test_every_manifest_has_deployment_and_service():
    for rel in _all_manifests():
        docs = _yaml_docs(rel)
        kinds = [d.get("kind", "") for d in docs]
        if "Service" in kinds:
            check("Deployment" in kinds,
                  f"{rel}: contient Deployment + Service", str(kinds))


def test_image_naming_convention():
    """All containers must use the '<service>:ft_services' image tag."""
    for rel in _all_manifests():
        docs = _yaml_docs(rel)
        for d in docs:
            if d.get("kind") != "Deployment":
                continue
            for c in d["spec"]["template"]["spec"]["containers"]:
                img = c.get("image", "")
                check(img.endswith(":ft_services"),
                      f"{rel}: image tag ':ft_services'", img)
                check(c.get("imagePullPolicy") == "Never",
                      f"{rel}: imagePullPolicy Never (image locale)",
                      str(c.get("imagePullPolicy")))


def test_ports_match_subject():
    """Ports in manifests must match the subject (5050 WP, 3306 MySQL, etc.)."""
    expected = {
        "nginx": [80, 443],
        "ftp": [21],
        "wordpress": [5050],
        "mysql": [3306],
        "phpmyadmin": [5000],
        "grafana": [3000],
        "influxdb": [8086],
        "ssh": [22],
    }
    for svc, ports in expected.items():
        rel = f"yaml/{svc}.yaml"
        if not path_exists(rel):
            rel = f"yaml/bonus/{svc}.yaml"
        docs = _yaml_docs(rel)
        for d in docs:
            if d.get("kind") == "Deployment":
                cports = []
                for c in d["spec"]["template"]["spec"]["containers"]:
                    cports.extend([p["containerPort"] for p in c.get("ports", [])])
                for p in ports:
                    check(p in cports, f"{svc}: containerPort {p} présent", str(cports))


def test_pvc_references():
    """Persistent services must reference their PVC."""
    for svc, pvc in (("wordpress", "wordpress-pvc"),
                     ("mysql", "mysql-pvc"),
                     ("grafana", "grafana-pvc"),
                     ("influxdb", "influxdb-pvc")):
        rel = f"yaml/{svc}.yaml" if path_exists(f"yaml/{svc}.yaml") else f"yaml/bonus/{svc}.yaml"
        docs = _yaml_docs(rel)
        text = read_file(rel)
        check(pvc in text, f"{svc}: référence le PVC {pvc}")


def test_pvc_file_defines_all_claims():
    docs = _yaml_docs("yaml/pvc.yaml")
    kinds = [d.get("kind") for d in docs]
    check(kinds.count("PersistentVolumeClaim") >= 4,
          "pvc.yaml: 4+ PVC définis", str(kinds))


def test_configmap_and_secret_present():
    check(path_exists("yaml/configmap.yaml"), "configmap.yaml présent")
    check(path_exists("yaml/secret.yaml.example"), "secret.yaml.example présent")
    cm = _yaml_docs("yaml/configmap.yaml")
    check(any(d.get("kind") == "ConfigMap" for d in cm), "configmap.yaml: ConfigMap")


def test_secret_is_not_committed_real():
    """The real secret.yaml must not exist (only the example template)."""
    expect(not path_exists("yaml/secret.yaml"),
           "le secret.yaml réel n'est pas commité (git-ignoré)")


def test_health_probes_present():
    """Deployments should define liveness + readiness probes."""
    for rel in _all_manifests():
        docs = _yaml_docs(rel)
        for d in docs:
            if d.get("kind") != "Deployment":
                continue
            for c in d["spec"]["template"]["spec"]["containers"]:
                has_liveness = "livenessProbe" in c
                has_readiness = "readinessProbe" in c
                check(has_liveness, f"{rel}: livenessProbe sur {c.get('name')}")
                check(has_readiness, f"{rel}: readinessProbe sur {c.get('name')}")


def test_metallb_manifest():
    docs = _yaml_docs("yaml/metallb.yaml")
    check(any(d.get("kind") == "MetalLB" for d in docs),
          "metallb.yaml: définit MetalLB (load balancer)")


def test_env_from_configmap_and_secret():
    """Deployments using credentials must pull from configMap + secret."""
    docs = _yaml_docs("yaml/wordpress.yaml")
    text = read_file("yaml/wordpress.yaml")
    check("configMapRef" in text, "wordpress: utilise configMapRef")
    check("secretRef" in text, "wordpress: utilise secretRef")


def test_service_types():
    """nginx, ftp, wordpress, grafana, ssh must be LoadBalancer (public)."""
    for svc in ("nginx", "ftp", "wordpress", "grafana", "ssh"):
        rel = f"yaml/{svc}.yaml" if path_exists(f"yaml/{svc}.yaml") else f"yaml/bonus/{svc}.yaml"
        docs = _yaml_docs(rel)
        for d in docs:
            if d.get("kind") == "Service":
                check(d["spec"]["type"] == "LoadBalancer",
                      f"{svc}: Service LoadBalancer (MetalLB)", d["spec"]["type"])
    # mysql + phpmyadmin + influxdb internal (ClusterIP)
    for svc in ("mysql", "phpmyadmin", "influxdb"):
        rel = f"yaml/{svc}.yaml" if path_exists(f"yaml/{svc}.yaml") else f"yaml/bonus/{svc}.yaml"
        docs = _yaml_docs(rel)
        for d in docs:
            if d.get("kind") == "Service":
                check(d["spec"]["type"] == "ClusterIP",
                      f"{svc}: Service ClusterIP (interne)", d["spec"]["type"])
