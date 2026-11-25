#!/bin/bash
# ============================================================================ #
#  ft_services — setup.sh (42 School)                                        #
#                                                                              #
#  Orchestrates the full Kubernetes cluster:                               #
#    - vm        : install prerequisites (docker, minikube, kubectl)         #
#    - minikube  : start the local cluster                                   #
#    - img       : build all custom Docker images (Alpine base)              #
#    - dep       : load images into minikube + apply manifests               #
#    - svc       : (alias) deploy everything                                #
#                                                                              #
#  Usage:                                                                    #
#    ./setup.sh                      # full stack (vm minikube img dep svc)  #
#    ./setup.sh x                    # skip VM setup (outside the 42 VM)     #
#    ./setup.sh minikube img         # only those stages                     #
#    ./setup.sh nginx wordpress      # build + deploy only those services    #
#    ./setup.sh vm minikube img dep svc   # explicit full sequence          #
# ============================================================================ #
set -e

# --- Project paths -----------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="${SCRIPT_DIR}/srcs"
YAML_DIR="${SCRIPT_DIR}/yaml"

# Services (name : Dockerfile dir : manifest)
SERVICES="nginx ftp wordpress mysql phpmyadmin grafana influxdb telegraf ssh"
MANDATORY="nginx ftp wordpress mysql phpmyadmin"
BONUS="grafana influxdb telegraf ssh"

# Colored logging
log()  { printf "\033[36m>>\033[0m %s\n" "$*"; }
ok()   { printf "\033[32m  OK\033[0m %s\n" "$*"; }
warn() { printf "\033[33m  WARN\033[0m %s\n" "$*"; }
die()  { printf "\033[31m  ERROR\033[0m %s\n" "$*" >&2; exit 1; }

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "required command not found: $1"
}

# --- Stage: VM prerequisites ---------------------------------------------------
stage_vm() {
    log "Stage VM: installing prerequisites..."
    require_cmd apt-get
    apt-get update -y
    apt-get install -y docker.io curl apt-transport-https ca-certificates
    systemctl enable --now docker 2>/dev/null || true
    # minikube + kubectl
    if ! command -v minikube >/dev/null 2>&1; then
        curl -LO "https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64" \
            -o /usr/local/bin/minikube
        chmod +x /usr/local/bin/minikube
    fi
    if ! command -v kubectl >/dev/null 2>&1; then
        curl -LO "https://storage.googleapis.com/kubernetes-release/release/latest/bin/linux/amd64/kubectl" \
            -o /usr/local/bin/kubectl
        chmod +x /usr/local/bin/kubectl
    fi
    ok "prerequisites installed (docker, minikube, kubectl)"
}

# --- Stage: start minikube ----------------------------------------------------
stage_minikube() {
    log "Stage minikube: starting cluster..."
    require_cmd minikube
    if minikube status >/dev/null 2>&1; then
        log "minikube already running."
    else
        minikube start --driver=docker --cpus=2 --memory=4096
    fi
    eval "$(minikube docker-env)"
    ok "minikube cluster ready"
}

# --- Stage: build images ------------------------------------------------------
stage_img() {
    log "Stage img: building custom Docker images (Alpine base)..."
    require_cmd docker
    for svc in $SERVICES; do
        df="${SRC_DIR}/${svc}/Dockerfile"
        if [ ! -f "$df" ]; then
            warn "no Dockerfile for '${svc}', skipping image build"
            continue
        fi
        log "building image ${svc}:ft_services ..."
        docker build -t "${svc}:ft_services" "${SRC_DIR}/${svc}"
        ok "image ${svc}:ft_services built"
    done
}

# --- Stage: load images + deploy manifests -------------------------------------
stage_dep() {
    log "Stage dep: loading images into minikube + applying manifests..."
    require_cmd kubectl
    for svc in $SERVICES; do
        if docker image inspect "${svc}:ft_services" >/dev/null 2>&1; then
            minikube image load "${svc}:ft_services"
        fi
    done
    # Base resources
    apply_yaml "${YAML_DIR}/metallb.yaml"
    apply_yaml "${YAML_DIR}/configmap.yaml"
    apply_yaml "${YAML_DIR}/pvc.yaml"
    # Secret (generated or user-provided)
    if [ -f "${YAML_DIR}/secret.yaml" ]; then
        apply_yaml "${YAML_DIR}/secret.yaml"
    else
        warn "no yaml/secret.yaml found — run 'make secrets' or copy the example"
    fi
    ok "base resources applied"
}

apply_yaml() {
    local file="$1"
    [ -f "$file" ] || { warn "manifest not found: ${file}"; return 0; }
    log "applying ${file}"
    kubectl apply -f "$file"
}

# --- Stage: deploy services (full or selected) -------------------------------
stage_svc() {
    log "Stage svc: deploying services..."
    require_cmd kubectl
    local to_deploy="$1"
    [ -z "$to_deploy" ] && to_deploy="$SERVICES"
    for svc in $to_deploy; do
        local manifest="${YAML_DIR}/${svc}.yaml"
        [ -f "$manifest" ] || manifest="${YAML_DIR}/bonus/${svc}.yaml"
        apply_yaml "$manifest"
        ok "deployed ${svc}"
    done
}

# --- Argument parsing ---------------------------------------------------------
main() {
    local args=("$@")
    local skip_vm=0
    local stages=""
    local services=""

    for a in "${args[@]}"; do
        case "$a" in
            x) skip_vm=1 ;;
            vm|minikube|img|dep|svc) stages="${stages} ${a}" ;;
            nginx|ftp|wordpress|mysql|phpmyadmin|grafana|influxdb|telegraf|ssh)
                services="${services} ${a}" ;;
            *) warn "unknown argument: ${a} (ignored)" ;;
        esac
    done

    # Default: full sequence
    if [ -z "$stages" ] && [ -z "$services" ]; then
        stages=" vm minikube img dep svc"
    fi

    # If only services are given, ensure the cluster + images are ready first
    if [ -z "$stages" ] && [ -n "$services" ]; then
        [ "$skip_vm" -eq 1 ] || stage_vm
        stage_minikube
        stage_img
        stage_dep
        stage_svc "$services"
        return 0
    fi

    # Run requested stages in order
    for stage in $stages; do
        case "$stage" in
            vm)       [ "$skip_vm" -eq 1 ] || stage_vm ;;
            minikube) stage_minikube ;;
            img)      stage_img ;;
            dep)      stage_dep ;;
            svc)      stage_svc "" ;;
        esac
    done

    log "All requested stages complete."
    log "Cluster access:"
    local ip
    ip="$(minikube ip 2>/dev/null || echo '<minikube-ip>')"
    log "  WordPress : http://${ip}:5050"
    log "  phpMyAdmin: https://${ip}/phpmyadmin"
    log "  Grafana   : http://${ip}:3000"
}

main "$@"
