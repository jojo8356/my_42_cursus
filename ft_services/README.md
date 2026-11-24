# ft_services — 42 School

Projet d'**administration système / DevOps** : déploiement automatique d'un
**cluster Kubernetes** (minikube + MetalLB) de micro-services, chaque service
dans son **pod dédié**, les images **construites depuis Alpine** avec
**Dockerfile maison** (pas d'images pré-faites de hub).

## Architecture

```
   Internet
      │
      ▼
  MetalLB (load balancer — une IP publique)
      │
      ├── Nginx (80/443, SSL) ── reverse proxy /phpmyadmin ──▶ phpMyAdmin (5000)
      │                      └── 307 redirect ──▶ WordPress (5050)
      ├── FTPS  (21, SSL)
      ├── WordPress (5050, nginx + php-fpm) ──▶ MySQL/MariaDB (3306)
      ├── phpMyAdmin (5000) ──▶ MySQL/MariaDB (3306)
      └── BONUS: Grafana (3000) ──▶ InfluxDB (8086) ◀── Telegraf
                              SSH (22)
```

## Services

### Obligatoires
| Service       | Rôle                                     | Ports |
|---------------|------------------------------------------|-------|
| Nginx         | SSL, reverse proxy phpMyAdmin, 307 → WP  | 80, 443 |
| FTPS          | FTP sécurisé (SSL)                       | 21 |
| WordPress     | CMS + php-fpm, WP-CLI, 2 users, PVC      | 5050 |
| MySQL/MariaDB | Base de données, PVC                     | 3306 |
| phpMyAdmin    | Administration de la base                | 5000 |

### Bonus
| Service    | Rôle                          | Ports |
|------------|-------------------------------|-------|
| Grafana    | Monitoring / visualisation    | 3000 |
| InfluxDB   | Base temporelle (pour Grafana)| 8086 |
| Telegraf   | Collecte de métriques → InfluxDB | — |
| SSH        | Accès distant sécurisé        | 22 |

## Respect des règles
- ✅ **Kubernetes** (minikube) + **MetalLB** (load balancer, une IP publique).
- ✅ Chaque service dans **son pod dédié**.
- ✅ Images **construites depuis Alpine** avec **Dockerfile maison** — **pas
  d'images pré-faites de hub**.
- ✅ Nginx : **SSL**, reverse proxy **phpMyAdmin**, **307 redirect** vers
  WordPress.
- ✅ WordPress : **WP-CLI**, **2 utilisateurs** (admin sans
  « admin/administrateur » = `webmaster`), **volume persistant**.
- ✅ MySQL : **volume persistant**, base + utilisateur d'application.
- ✅ Configuration **ConfigMap** (non-sensible) + **Secret** (sensible).
- ✅ **Health checks** / **liveness & readiness probes** sur tous les services.
- ✅ **Bonus** : Grafana, InfluxDB, Telegraf, SSH.

## Prérequis
- Docker Engine
- minikube + kubectl
- Linux (le script `setup.sh` installe les prérequis sur la VM 42)

## Utilisation

```bash
cd ft_services

# Générer les secrets (credentials)
cp yaml/secret.yaml.example yaml/secret.yaml
# puis éditer yaml/secret.yaml avec les vrais mots de passe
# (ou automatiser via le générateur — voir tests)

# Déploiement complet (installe prérequis + minikube + images + déploiement)
./setup.sh

# Hors VM 42 (prérequis déjà installés)
./setup.sh x

# Séquence explicite
./setup.sh vm minikube img dep svc

# Déployer uniquement certains services
./setup.sh nginx wordpress
```

Après `./setup.sh`, accédez aux services via l'IP MetalLB :
- WordPress : `http://<IP>:5050`
- phpMyAdmin : `https://<IP>/phpmyadmin`
- Grafana : `http://<IP>:3000`

## Structure

```
ft_services/
├── setup.sh                 # orchestration (vm/minikube/img/dep/svc)
├── subject.md               # énoncé synthétisé
├── README.md
├── .gitignore
├── srcs/                    # Dockerfiles + configs (Alpine base)
│   ├── nginx/               # Dockerfile, nginx.conf, entrypoint.sh
│   ├── ftp/                 # Dockerfile, proftpd.conf, entrypoint.sh
│   ├── wordpress/          # Dockerfile, wordpress.conf, entrypoint.sh
│   ├── mysql/               # Dockerfile, init.sql, entrypoint.sh
│   ├── phpmyadmin/          # Dockerfile, nginx.conf, config.inc.php, entrypoint.sh
│   └── bonus/
│       ├── grafana/         # Dockerfile
│       ├── influxdb/        # Dockerfile
│       ├── telegraf/        # Dockerfile, telegraf.conf
│       └── ssh/             # Dockerfile, entrypoint.sh
├── yaml/                    # manifestes Kubernetes
│   ├── metallb.yaml
│   ├── configmap.yaml
│   ├── secret.yaml.example  # template (le secret.yaml réel est git-ignoré)
│   ├── pvc.yaml
│   ├── nginx.yaml / ftp.yaml / wordpress.yaml / mysql.yaml / phpmyadmin.yaml
│   └── bonus/               # grafana.yaml / influxdb.yaml / telegraf.yaml / ssh.yaml
└── tests/                   # suite de tests complète
```

## Tests

La suite de tests complète valide **staticamente** toute la configuration
(Docker/minikube/kubectl absents dans cet environnement — validation par
analyse structurelle, syntaxe shell, YAML, Dockerfiles, règles du sujet) :

```bash
python3 tests/run_tests.py
```

Elle couvre : syntaxe des scripts shell (`bash -n`), tous les Dockerfiles
(base Alpine, pas de `latest`, pas d'image pré-faite, commande réelle), la
configuration nginx (SSL, 307 redirect, reverse proxy phpMyAdmin), WordPress
(WP-CLI, 2 users, PVC), MySQL (init.sql, PVC), phpMyAdmin, les manifestes
Kubernetes (services, ports, PVC, ConfigMap, Secret, probes), `setup.sh`
(stages, services, arguments), et tous les bonus.
