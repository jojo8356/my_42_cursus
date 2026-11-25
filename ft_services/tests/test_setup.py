# ============================================================================ #
#  Tests — setup.sh orchestration script                                     #
# ============================================================================ #
from lib import check, expect, read_file, bash_syntax_ok


def _setup():
    return read_file("setup.sh")


def test_setup_syntax_valid():
    """setup.sh must pass bash -n (no syntax errors)."""
    ok, err = bash_syntax_ok("setup.sh")
    check(ok, "setup.sh: syntaxe shell valide (bash -n)", err)


def test_setup_has_all_stages():
    """The script must define vm, minikube, img, dep, svc stages."""
    txt = _setup()
    for stage in ("stage_vm", "stage_minikube", "stage_img", "stage_dep", "stage_svc"):
        check(stage in txt, f"setup.sh: fonction {stage} définie")


def test_setup_handles_all_services():
    """The service list must include all mandatory + bonus services."""
    txt = _setup()
    for svc in ("nginx", "ftp", "wordpress", "mysql", "phpmyadmin",
                "grafana", "influxdb", "telegraf", "ssh"):
        check(svc in txt, f"setup.sh: service '{svc}' géré")


def test_setup_builds_images():
    """The img stage must build images with docker build."""
    txt = _setup()
    check("docker build" in txt, "setup.sh: utilise 'docker build'")
    check(":ft_services" in txt, "setup.sh: tags les images ':ft_services'")


def test_setup_applies_manifests():
    """The dep/svc stages must apply Kubernetes manifests via kubectl."""
    txt = _setup()
    check("kubectl apply" in txt, "setup.sh: utilise 'kubectl apply'")
    check("minikube" in txt, "setup.sh: gère minikube")


def test_setup_skip_vm_flag():
    """The 'x' argument must skip VM setup."""
    txt = _setup()
    check('"x"' in txt or "skip_vm" in txt, "setup.sh: gère l'argument 'x' (skip VM)")


def test_setup_default_full_sequence():
    """With no args, the script runs the full sequence (vm minikube img dep svc)."""
    txt = _setup()
    check("vm minikube img dep svc" in txt,
          "setup.sh: séquence par défaut complète définie")


def test_setup_has_shebang_and_strict_mode():
    txt = _setup()
    check(txt.startswith("#!/bin/bash"), "setup.sh: shebang bash")
    check("set -e" in txt, "setup.sh: mode strict (set -e)")


def test_setup_loads_images_into_minikube():
    txt = _setup()
    check("minikube image load" in txt, "setup.sh: charge les images dans minikube")
