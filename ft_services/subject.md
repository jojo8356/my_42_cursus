# ft_services — Sujet (énoncé synthétisé le plus complet)

## Contexte
Projet d'**administration système / DevOps** à l'école 42. L'objectif est de
déployer **automatiquement un cluster de micro-services** en utilisant
**Kubernetes** (via **minikube**) et des **images Docker maison**, afin de
virtualiser un réseau et de mettre en place un environnement de production.

## Orchestration
- **Kubernetes** (minikube) orchestra les conteneurs et leurs interconnexions.
- **MetalLB** fait office de **load balancer** : il publie **une IP publique**
  pour l'ensemble du cluster.
- Chaque service tourne dans **son pod dédié**.
- Toutes les images sont **construites depuis **Alpine Linux** avec un
  **Dockerfile maison** — **pas d'images pré-faites** provenant d'un hub
  (seule la base Alpine est utilisée).

## Services à déployer

### Obligatoires
1. **Nginx** — serveur web :
   - SSL / HTTPS
   - reverse proxy vers **phpMyAdmin** (URI `/phpmyadmin`)
   - **redirection 307** vers le container **WordPress**
   - écoute HTTP (80) + HTTPS (443)
2. **FTPS** — serveur de fichiers sécurisé (FTP sur SSL), port **21**.
3. **WordPress** — CMS + php-fpm, port **5050** :
   - installation automatisée via **WP-CLI**
   - **2 utilisateurs** (admin sans « admin/administrateur »)
   - **volume persistant** pour les fichiers (`wp-content`)
4. **MySQL / MariaDB** — base de données pour WordPress, port **3306** :
   - **volume persistant** pour `/var/lib/mysql`
   - création de la base + de l'utilisateur d'application
5. **phpMyAdmin** — interface d'administration de la base, port **5000**.

### Bonus
- **Grafana** — visualisation / monitoring, port **3000** (volume persistant).
- **InfluxDB** — base de données temporelle pour Grafana, port **8086**
  (volume persistant).
- **Telegraf** — collecte de métriques (système + docker) → InfluxDB.
- **SSH** — accès distant sécurisé, port **22**.
- **Health checks** / **liveness & readiness probes** sur tous les services.
- **Volumes persistants** (PVC) pour WordPress, MySQL, Grafana, InfluxDB.
- **SSL/TLS** partout (nginx, FTPS).

## Configuration & accès
- Configuration **non-sensible** via **ConfigMap**.
- Configuration **sensible** (mots de passe) via **Secret**.
- Un script **`setup.sh`** orchestre tout : installation des prérequis,
  démarrage de minikube, build des images, déploiement des manifestes.

## Ports récapitulatifs
| Service       | Port |
|---------------|------|
| Nginx         | 80, 443 |
| FTPS          | 21 |
| WordPress     | 5050 |
| MySQL/MariaDB | 3306 |
| phpMyAdmin    | 5000 |
| Grafana       | 3000 |
| InfluxDB      | 8086 |
| SSH           | 22 |
