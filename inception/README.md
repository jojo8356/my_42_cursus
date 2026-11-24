# Inception — 42 School

Projet d'administration système : mise en place d'une **infrastructure web
complete** avec **Docker Compose** — NGINX (TLS) + WordPress/php-fpm + MariaDB —
et **bonus** (Adminer, Redis, FTP, site web personnel).

## Architecture

```
                    Internet
                        │
                    ┌───▼────┐
                    │ NGINX  │  ← seule entrée publique, TLS 1.2/1.3, port 443
                    └───┬────┘
             fastcgi     │      volume partagé (ro)
        wordpress:9000  │      /var/www/html
                    ┌───▼────┐
                    │ WordPress│  php-fpm TCP :9000, pas de nginx
                    └───┬────┘
                 mysql  │
                    ┌───▼────┐
                    │MariaDB │  MySQL/MariaDB seul, :3306, pas de nginx
                    └────────┘

  Réseau dédié (bridge) : "inception"   •   Volumes nommés : vol-mariadb, vol-wordpress
```

- **NGINX** — reverse proxy TLS, seule porte publique (`443`, TLS 1.2/1.3).
  Génère un certificat auto-signé si absent. Transfère le PHP vers
  `wordpress:9000` via FastCGI et sert les fichiers WordPress.
- **WordPress + php-fpm** — application sans nginx, php-fpm écoute sur **TCP
  `0.0.0.0:9000`** (communication inter-containers). Télécharge WordPress depuis
  wordpress.org (pas d'image pré-faite). L'entrypoint attend MariaDB, installe
  WordPress et crée **2 utilisateurs**.
- **MariaDB** — serveur de base de données seul (pas de nginx), config sur
  `/var/lib/mysql`, mots de passe via **Docker secrets**.

## Règles du sujet (respectées)

- ✅ Images **construites** depuis **Debian (penultimate stable : bookworm)**,
  tag versionné — **jamais `latest`**, **pas d'images pré-faites de DockerHub**.
- ✅ **NGINX** : seule entrée publique, **TLS 1.2/1.3**, port **443**.
- ✅ **WordPress** et **MariaDB** **sans nginx**.
- ✅ **docker compose** obligatoire, piloté par un **Makefile**.
- ✅ **`.env`** obligatoire (configuration non-sensible).
- ✅ **2 volumes nommés persistants** (DB + fichiers WP), pinnés sous
  `/home/$USER/data` via `driver_opts` — **pas de bind mount direct**.
- ✅ **Réseau Docker dédié** (bridge), **pas de `network: host`**, **pas de
  `links:`**.
- ✅ **`restart: unless-stopped`** sur tous les services.
- ✅ **2 utilisateurs WordPress** dont **admin sans « admin/administrateur »**
  (ici `inceptor`).
- ✅ **Credentials via Docker secrets** (`secrets/*.txt`, jamais en env ni en git).

## Bonus implémentés

| Service    | Rôle                                         | Portique        |
|------------|----------------------------------------------|-----------------|
| Adminer    | Interface d'administration de la base        | `8080`          |
| Redis      | Cache / sessions en mémoire                  | `6379`          |
| FTP        | Serveur FTP (vsftpd)                         | `21`            |
| Website    | Site web personnel statique (NGINX)          | `8081`          |

Les bonus sont **opt-in** via le profil Compose `bonus`.

## Prérequis

- Docker Engine + Docker Compose plugin
- `make`
- Python 3 (uniquement pour la suite de tests / validation)

## Utilisation

```bash
cd inception

# Générer les secrets (credentials)
make secrets-generate

# Lancement du stack obligatoire
make up

# Lancement avec les bonus
make up-bonus

# Aide, état, logs
make help
make ps
make logs            # make logs FOLLOW=1 pour suivre

# Rebuild complet
make re

# Arrêt / nettoyage
make down
make clean
```

Après `make up` :
- Site WordPress : `https://<DOMAIN_NAME>` (TLS auto-signé — accepter le
  certificat dans le navigateur).
- Admin WordPress : utilisateur `inceptor` (mot de passe dans
  `secrets/wp_admin_password.txt`).

## Tests

La suite de tests complète valide **staticamente** toute la configuration
(Docker absent dans cet environnement — validation par analyse structurelle et
YAML) :

```bash
make test
# ou
python3 tests/run_tests.py
```

Elle couvre : syntaxe YAML, respect de chaque règle du sujet, Dockerfiles
(base, pas de `latest`, pas d'image pré-faite, commande réelle), config NGINX
(TLS 1.2/1.3, 443, fastcgi), php-fpm (TCP 9000), MariaDB, Makefile, `.env`,
secrets, et tous les services bonus.

## Structure

```
inception/
├── Makefile
├── README.md
├── .gitignore
├── secrets/                    # credentials (git-ignorés)
├── srcs/
│   ├── .env
│   ├── docker-compose.yml
│   └── requirements/
│       ├── nginx/             # Dockerfile, conf/nginx.conf, tools/entrypoint.sh
│       ├── wordpress/         # Dockerfile, conf/php-fpm.conf, tools/entrypoint.sh
│       ├── mariadb/           # Dockerfile, conf/my.cnf, tools/entrypoint.sh
│       └── bonus/
│           ├── adminer/       # Dockerfile, tools/entrypoint.sh
│           ├── redis/         # Dockerfile
│           ├── ftp/           # Dockerfile, conf/vsftpd.conf, tools/entrypoint.sh
│           └── website/       # Dockerfile, html/index.html
└── tests/                     # suite de tests complète
```
