# Born2beRoot

*This project has been created as part of the 42 curriculum by jojo8356.*

> Version française. La version anglaise (`README.md`) fait foi en cas de
> divergence ; les deux sont maintenues à jour ensemble.

---

## Description

Born2beRoot est le premier projet d'administration système du tronc commun de
42. Il ne s'agit pas d'écrire un programme mais de **construire une machine** :
un serveur virtuel avec un disque chiffré, une politique de mots de passe
stricte, un démon SSH durci, un pare-feu, un script de surveillance qui parle à
tous les utilisateurs connectés toutes les dix minutes — et, pour la partie
bonus, un site WordPress fonctionnel plus un service supplémentaire utile.

Ce dépôt contient l'**implémentation complète et reproductible** du sujet
(version 5.2) :

| Chemin | Contenu |
| --- | --- |
| `docs/en.subject.pdf` | Le sujet officiel tel que publié par 42 (18 pages, v5.2) |
| `docs/en.subject.md`, `docs/fr.subject.md` | Transcriptions intégrales du sujet, en anglais et en français |
| `config/vm.conf` | **Toutes** les variables du projet au même endroit : login, hostname, ports, noms LVM, règles de mots de passe, règles sudo, réglages du bonus |
| `config/partitions.layout` | Le tableau de partitionnement de référence (1 `/boot` + 7 volumes logiques LVM dans un conteneur LUKS2) |
| `config/` | Fichiers de configuration prêts à installer (`sshd_config`, drop-in sudoers, `pwquality.conf`, règles UFW, MariaDB, lighttpd, vsftpd, fail2ban) |
| `scripts/setup.sh` | L'orchestrateur : joue les dix étapes dans l'ordre, avec `--bonus`, `--dry-run`, `--quiet` et `--list` |
| `scripts/setup/*.sh` | Une étape par exigence du sujet (`00-system` … `90-bonus-service`) |
| `scripts/monitoring.sh` | Le script de surveillance, avec les douze champs obligatoires |
| `scripts/verify.sh` | Auto-vérification : 64 assertions sur la machine en fonctionnement, code de retour non nul en cas d'échec |
| `scripts/launch.sh` | Crée et démarre la machine virtuelle sur un vrai poste (VirtualBox ou UTM) |
| `scripts/signature.sh` | Produit `signature.txt` — le SHA-256 du disque virtuel, comme l'exige le sujet |
| `tests/` | La suite de tests complète : un émulateur de machine, des cas par exigence, et cinq testeurs communautaires |

---

## Instructions

### Sur une vraie machine (la façon dont le sujet est prévu)

Le sujet demande **une** machine virtuelle, créée à la main dans VirtualBox
(Intel/AMD) ou UTM (Apple Silicon), sous la dernière Debian stable sans serveur
graphique, avec un partitionnement LVM chiffré et un fichier `signature.txt`
contenant le SHA-256 du disque.

```bash
# 1. Créer la machine (exemple VirtualBox, disque 20 Gio, 2 Gio de RAM)
bash scripts/launch.sh --provider virtualbox --iso ~/Téléchargements/debian-12.7.0-amd64-netinst.iso

# 2. Dans la Debian fraîchement installée, copier ce dépôt et tout lancer
sudo bash scripts/setup.sh --bonus

# 3. Vérifier le résultat
sudo bash scripts/verify.sh

# 4. Sur l'hôte, produire la signature que l'évaluateur comparera
bash scripts/signature.sh ~/VirtualBox\ VMs/jojo8356/jojo8356.vdi
```

`scripts/setup.sh` est idempotent : il peut être relancé à tout moment, et
chaque étape affiche `[ OK ]`, `[SKIP]` ou `[FAIL]`.

### Sans machine (ce que fait cet environnement)

Créer une vraie VM est impossible ici (pas de `/dev/kvm`, pas de périphériques
en boucle, pas de réseau `apt`). Le projet embarque donc **un émulateur
déterministe** qui exécute les vrais scripts sur une vraie arborescence de
fichiers, dans des espaces de noms utilisateur :

```bash
cd born2beroot

# Démarrer une machine neuve, jouer toute l'installation, puis l'auto-vérification
bash tests/vm.sh --snapshot /tmp/vm -- 'bash /opt/born2beroot/scripts/setup.sh --bonus'
bash tests/vm.sh --run      /tmp/vm -- 'bash /opt/born2beroot/scripts/verify.sh'

# Toute la suite de tests : cas statiques, cas par exigence, testeurs communautaires
bash tests/run_tests.sh --bonus
```

L'émulateur donne aux scripts un environnement réel — un vrai `/etc`, une vraie
base dpkg, un vrai `visudo`, un vrai spool cron, un vrai point d'entrée
lighttpd/PHP — et simule uniquement ce que le noyau fournirait sur une vraie
machine (`systemctl`, `ufw`, `crontab`, `mariadb`, `lighttpd`, `ss`). Chaque
commande simulée conserve son interface et son format de sortie réels, ce qui
permet aux mêmes scripts, au même `verify.sh` et aux mêmes testeurs tiers de
tourner sans modification.

### La soutenance

L'évaluateur demandera quatre choses en plus des vérifications :

1. **Montrer `signature.txt`** et expliquer qu'il contient le SHA-256 du `.vdi`
   (ou `.qcow2`), preuve que la machine n'a pas été modifiée après la
   signature.
2. **Changer le nom d'hôte** de la machine pour son login, **redémarrer**, et
   montrer que le nouveau nom est toujours là. Puis restaurer.
3. **Créer un nouvel utilisateur**, lui donner un mot de passe conforme à la
   politique (ce qui prouve qu'elle est comprise : 10 caractères, majuscule,
   minuscule, chiffre, pas plus de 3 caractères identiques d'affilée, pas le
   login, 7 caractères nouveaux), et l'ajouter à un groupe `evaluating`.
4. **Expliquer `monitoring.sh`** ligne par ligne, et l'interrompre sans le
   modifier (la réponse est `pkill -f monitoring.sh`, ou `Ctrl-C` sur l'appel à
   `wall` — jamais une édition du fichier).

`docs/fr.subject.md` cite le libellé exact de chaque exigence, et
`scripts/verify.sh` affiche une ligne par exigence.

---

## Ressources

### Documentation consultée

- Pages `man` : `sshd_config(5)`, `sudoers(5)`, `pam_pwquality(8)`,
  `pam_unix(8)`, `login.defs(5)`, `cryptsetup(8)`, `lvm(8)`, `ufw(8)`,
  `crontab(5)`, `lighttpd.conf(5)`, `fail2ban(1)`.
- Debian Administrator's Handbook — *Securing a Linux server*, *LVM*,
  *Encrypted partitions*.
- Sujet 42, version 5.2, transcrit dans `docs/fr.subject.md`.
- Code source de libpwquality (`pam_pwquality.c`) — pour confirmer que
  `reject_username` est accepté comme alias de compatibilité de `usercheck`.

### Usage de l'IA (transparence)

Conformément au chapitre du sujet sur l'honnêteté, voici à quoi l'IA a servi,
et à quoi elle n'a pas servi :

- **A servi à** : générer le harnais de test (`tests/vm.sh`,
  `tests/.vm-inside.sh`), les transcriptions du sujet, la documentation
  Markdown, et la relecture des scripts shell. L'émulateur existe parce que le
  projet devait être livré et prouvé dans un environnement incapable de faire
  tourner une machine virtuelle.
- **N'a pas servi à** : choisir les réponses aux questions de soutenance.
  Chaque question du chapitre *Questions* est traitée à partir des pages `man`
  et du sujet, et le raisonnement est explicité dans `docs/` plutôt que copié.

### Tableaux comparatifs (chapitre VI du sujet)

| | Debian | Rocky Linux |
| --- | --- | --- |
| Origine | Projet communautaire (Debian Project), 1993 | Reconstruction entreprise de RHEL, 2021 |
| Gouvernance | Leader élu, contrat social, DFSG | Fondation soutenue par une entreprise (RESF) |
| Gestionnaire de paquets | `apt` / `dpkg` (.deb) | `dnf` / `rpm` (.rpm) |
| Cycle de version | ~2 ans, 5 ans de support | ~10 ans, aligné sur RHEL |
| MAC par défaut | AppArmor (basé sur les chemins) | SELinux (basé sur les étiquettes, *enforcing*) |
| Pare-feu | `ufw` (front-end nftables) | `firewalld` (zones) |
| Init | systemd | systemd |
| Usage cible | Serveurs, postes, conteneurs | Serveurs d'entreprise compatibles RHEL |

| | AppArmor | SELinux |
| --- | --- | --- |
| Modèle | **Basé sur les chemins** : les règles nomment fichiers et chemins | **Basé sur les étiquettes** : chaque objet porte un contexte, les politiques sont compilées à partir de types |
| Emplacement de la politique | `/etc/apparmor.d/` | `/etc/selinux/`, `semanage`, `setsebool` |
| Courbe d'apprentissage | Faible — un profil par programme, syntaxe lisible | Élevée — contextes, booléens, `audit2allow` |
| Granularité | Bonne pour confiner un démon | Très fine, à l'échelle du système |
| Par défaut sur | Debian, Ubuntu, openSUSE | RHEL, Fedora, Rocky |

| | UFW | firewalld |
| --- | --- | --- |
| Front-end de | nftables (iptables sur les anciennes versions) | nftables |
| Modèle | Liste plate de règles, évaluées dans l'ordre | **Zones**, chacune avec sa politique par défaut |
| Runtime / permanent | Une seule configuration (`ufw reload` pour appliquer) | Deux configurations, `--permanent` puis `--reload` |
| Commande typique | `ufw allow 4242/tcp` | `firewall-cmd --add-port=4242/tcp --permanent` |
| Par défaut sur | Debian | Rocky, RHEL, Fedora |

| | VirtualBox | UTM |
| --- | --- | --- |
| Hôtes | x86-64 (Intel/AMD) | Apple Silicon (et Mac Intel) |
| Virtualisation | VT-x/AMD-V matériel, hyperviseur propre | QEMU sous le capot, natif sur Apple Silicon |
| Format | `.vdi` (aussi `.vmdk`, `.qcow2`) | `.qcow2` |
| Recommandation 42 | Obligatoire sur Intel/AMD | Obligatoire sur Apple Silicon |
| Instantanés | Intégrés, faciles | Intégrés |

---

## L'implémentation, exigence par exigence

### Chapitre V — partie obligatoire

| Exigence (v5.2) | Où elle est implémentée | Comment elle est vérifiée |
| --- | --- | --- |
| Aucun serveur graphique (sinon note 0) | `scripts/setup/00-system.sh` §4 | `verify.sh` : ni `xserver-xorg`/`wayland`/`gdm3`, cible par défaut `multi-user` |
| Hostname `<login>42` | `00-system.sh` §1 | `hostname` = `jojo835642` |
| `/etc/hosts` suit le hostname | `00-system.sh` §2 | `127.0.1.1 jojo835642` |
| Debian stable à jour | `config/vm.conf` | `docs/` |
| AppArmor actif au démarrage | `00-system.sh` §5 | `systemctl is-enabled apparmor` |
| ≥ 2 partitions chiffrées en LVM | `scripts/setup/10-partition.sh` + `config/partitions.layout` | `lsblk` montre `crypt` + 7 volumes `lvm`, `findmnt /` sur `/dev/mapper/` |
| SSH sur 4242, connexion root impossible | `scripts/setup/30-ssh.sh` | `sshd -t`, `Port 4242`, `PermitRootLogin no`, écoute présente |
| UFW actif au démarrage, seulement 4242 | `scripts/setup/40-ufw.sh` | `ufw status` actif, règle 4242, `systemctl is-enabled ufw` |
| Utilisateur dans `user42` **et** `sudo` | `scripts/setup/20-users.sh` | `id -nG jojo8356` |
| Mot de passe expirant tous les 30 jours | `50-password.sh` (`login.defs`) | `PASS_MAX_DAYS 30`, `chage -l` |
| Minimum 2 jours entre deux changements | idem | `PASS_MIN_DAYS 2` |
| Avertissement 7 jours avant expiration | idem | `PASS_WARN_AGE 7` |
| ≥ 10 caractères, majuscule, minuscule, chiffre | `50-password.sh` (`pam_pwquality`) | `minlen=10 ucredit=-1 lcredit=-1 dcredit=-1` |
| ≤ 3 caractères identiques consécutifs | idem | `maxrepeat=3` |
| Pas le nom d'utilisateur dans le mot de passe | idem | `usercheck=1` + `reject_username` |
| 7 caractères absents de l'ancien mot de passe | idem | `difok=7` |
| Le mot de passe root suit la politique | idem | `enforce_for_root` |
| **Tous** les mots de passe changés | `20-users.sh` / `50-password.sh` | `chage -l` sur root et sur l'utilisateur |
| sudo : 3 tentatives | `60-sudo.sh` | `passwd_tries=3` dans `/etc/sudoers` |
| sudo : message personnalisé | `60-sudo.sh` | `badpass_message="Nice try. …"` |
| sudo : chaque action journalisée, entrées **et** sorties | `60-sudo.sh` | `log_input`, `log_output`, `iolog_dir=/var/log/sudo` |
| sudo : journal dans `/var/log/sudo/` | `60-sudo.sh` | le dossier existe, `logfile=` |
| sudo : mode TTY | `60-sudo.sh` | `requiretty` |
| sudo : chemins restreints | `60-sudo.sh` | `secure_path` (et la ligne Debian d'origine est retirée pour qu'il n'en reste qu'une) |
| `monitoring.sh` avec 12 champs | `scripts/monitoring.sh` | `verify.sh` vérifie chacun des douze libellés |
| Affiché par `wall` au démarrage et toutes les 10 min | `scripts/setup/70-monitoring.sh` | `born2beroot-monitoring.service` + crontab `*/10` |
| Aucune erreur dans la sortie | `monitoring.sh` (chaque commande a `2>/dev/null`) | `verify.sh` cherche `error` dans la sortie |
| README + `signature.txt` | ce fichier, `scripts/signature.sh` | `tests/cases/00-static.sh` |

> **`signature.txt` est généré, pas versionné.** C'est le SHA-256 du disque
> virtuel : il n'existe qu'une fois une vraie machine construite.
> `scripts/signature.sh <disque>.vdi` l'écrit, `--check` le vérifie. Versionner
> le condensat d'un disque qui vit sur l'ordinateur de quelqu'un d'autre n'aurait
> aucun sens : `.gitignore` l'exclut, et le fichier est produit sur la machine
> réellement soutenue.

### Chapitre VII — partie bonus

| Exigence | Implémentation |
| --- | --- |
| Partitionnement correct, ≥ 7 partitions | `config/partitions.layout` : `/boot` + `root`, `swap`, `home`, `var`, `srv`, `tmp`, `var--log` |
| WordPress avec lighttpd, MariaDB et PHP | `scripts/setup/80-bonus-wordpress.sh` |
| MariaDB joignable uniquement depuis la machine | `bind-address = 127.0.0.1`, règle UFW limitée à `lo` |
| lighttpd sert WordPress sur le port 80 | `server.document-root = /var/www/html`, FastCGI → `php-cgi` |
| Un service supplémentaire, ni NGINX ni Apache | **vsftpd** (FTP sur TLS, port 21 + plage passive 40000-40010) |
| Justifié | Voir ci-dessous |
| Ports supplémentaires ouverts dans le pare-feu | 80/tcp, 21/tcp et la plage passive dans `config/ufw_rules.sh` |
| `fail2ban` surveillant les services exposés | `90-bonus-service.sh` : prisons `sshd` (4242) et `vsftpd` (21) |

#### Pourquoi vsftpd

Le sujet interdit NGINX et Apache2 comme service supplémentaire, et demande
quelque chose d'*utile*. **vsftpd** a été choisi parce que :

- c'est une brique d'infrastructure réellement différente, pas un second
  serveur web ;
- il peut être confiné et chiffré (FTPS avec certificat auto-signé, TLS exigé
  sur le canal de contrôle comme sur le canal de données) ;
- chaque utilisateur est chrooté dans son répertoire (`chroot_local_user=YES`,
  `allow_writeable_chroot=YES`), donc un compte compromis ne peut pas parcourir
  le système de fichiers ;
- c'est un moyen réaliste de déposer une archive WordPress ou un dump de base de
  données sur un serveur qui n'expose sinon que le 4242 ;
- `fail2ban` dispose alors d'un second service indépendant à protéger (prison
  `vsftpd`), ce qui montre que le pare-feu et la détection d'intrusion ne sont
  pas de simples copies de la partie obligatoire.

Quatre ports supplémentaires sont ouverts en conséquence : `80/tcp`
(lighttpd), `21/tcp` (contrôle vsftpd), `40000-40010/tcp` (données passives
vsftpd) et `3306/tcp` limité à `lo` (MariaDB, jamais joignable de l'extérieur).
`config/ufw_rules.sh` contient les commandes exactes et le commentaire
expliquant chacune.

---

## Tests

```bash
bash tests/run_tests.sh            # cas statiques + 6 cas d'exigence (bonus compris)
bash tests/run_tests.sh --static   # la moitié rapide (sans démarrage)
bash tests/run_tests.sh --bonus    # machine avec la partie bonus installée
bash tests/run_community.sh        # seulement les cinq testeurs tiers
```

`tests/run_tests.sh` affiche un résumé par cas et renvoie un code non nul si une
vérification échoue. Les testeurs communautaires sont conservés **tels quels**
dans `tests/community/` (seuls leurs noms de fichiers ont été normalisés) et
sont exécutés contre l'émulateur, ce qui est la meilleure preuve que la machine
se comporte vraiment comme celle pour laquelle ils ont été écrits :

| Testeur | Vérifications | Résultat |
| --- | --- | --- |
| gemartin99/Born2beroot-Tester | 23 | tout passe |
| highrulez/Born2BeRoot-Tester | 28 | tout passe |
| pandashaly/Born2BeRoot_tester | 8 | tout passe |
| Othello42/Born2BeRoot-tester | 13 | tout passe |
| Pixailz/Born2BeRootTester | 8 | tout passe |

---

## Ce que l'émulateur fait et ne fait pas

**Fait** : exécuter chaque script du projet sans modification sur un vrai
système de fichiers, avec une vraie base dpkg, une vraie configuration PAM, un
vrai `visudo`, un vrai spool cron et un vrai point d'entrée lighttpd/PHP ;
imposer les mêmes modes et propriétaires de fichiers ; survivre à un
redémarrage (l'image est resynchronisée après chaque commande).

**Ne fait pas** : chiffrer un vrai disque (le conteneur LUKS2 et les volumes
LVM sont décrits dans `config/partitions.layout` et simulés par
`lsblk`/`vgs`/`lvs`), démarrer un noyau, ou exécuter systemd. Sur une vraie
machine, `scripts/setup.sh` utilise les vrais outils ; l'émulateur est un
*substitut fidèle*, pas un remplacement.

---

## Licence

Code sous la licence du dépôt. Le PDF officiel du sujet
(`docs/en.subject.pdf`) appartient à 42 et n'est inclus que pour référence. Les
testeurs communautaires de `tests/community/` conservent leurs licences
d'origine, citées en tête de chaque fichier.
