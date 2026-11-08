# Born2beRoot — Sujet officiel 42 (Version 5.2, Français)

> Transcription Markdown fidèle du PDF officiel `docs/en.subject.pdf` (18 pages, *Born2beRoot*, **Version : 5.2**).
> Le PDF est le document normatif ; cette transcription facilite la recherche, l'accessibilité et la comparaison.

**Résumé :** Ce document est un exercice de programmation système / administration système.

---

## Sommaire

| Chapitre | Titre | Page |
| :--- | :--- | :--- |
| I | Préambule | 2 |
| II | Introduction | 3 |
| III | Instructions IA | 4 |
| IV | Règles générales | 6 |
| V | Partie obligatoire | 7 |
| VI | Exigences du README | 12 |
| VII | Partie bonus | 14 |
| VIII | Soumission et évaluation par les pairs | 16 |

---

## Chapitre I — Préambule

*(Ce chapitre est volontairement laissé vide par 42.)*

---

## Chapitre II — Introduction

Ce projet a pour but de vous faire découvrir le merveilleux monde de la **virtualisation**.

Vous allez créer votre première machine dans **VirtualBox** (ou **UTM** si vous ne pouvez pas utiliser VirtualBox) en suivant des instructions précises. À la fin de ce projet, vous saurez installer votre propre système d'exploitation en appliquant des règles strictes.

---

## Chapitre III — Instructions IA

### ● Contexte

Ce projet est conçu pour vous aider à découvrir les briques fondamentales de votre formation 42.

Pour ancrer correctement les connaissances et compétences clés, il est essentiel d'adopter une approche réfléchie de l'usage des outils et supports d'IA.

Un véritable apprentissage fondamental exige un effort intellectuel réel — par le défi, la répétition et les échanges d'apprentissage entre pairs.

Pour un panorama plus complet de notre position sur l'IA — comme outil d'apprentissage, comme partie de la formation 42, et comme attente du marché du travail — reportez-vous à la FAQ dédiée sur l'intranet.

### ● Message principal

- ☛ Construire des bases solides sans raccourcis.
- ☛ Développer réellement les compétences techniques et humaines.
- ☛ Vivre un véritable apprentissage par les pairs, apprendre à apprendre et à résoudre de nouveaux problèmes.
- ☛ Le cheminement d'apprentissage compte plus que le résultat.
- ☛ Comprendre les risques liés à l'IA et développer des pratiques de contrôle et des contre-mesures efficaces pour éviter les pièges courants.

### ● Règles pour l'apprenant

- Vous devez raisonner sur les tâches qui vous sont confiées, surtout avant de faire appel à l'IA.
- Vous ne devez pas demander de réponses directes à l'IA.
- Vous devez vous renseigner sur l'approche globale de 42 concernant l'IA.

### ● Résultats de la phase

Dans cette phase fondamentale, vous obtiendrez les résultats suivants :

- Acquérir de bonnes bases techniques.
- Comprendre pourquoi et comment l'IA peut être dangereuse durant cette phase.

### ● Commentaires et exemples

- Oui, nous savons que l'IA existe — et oui, elle peut résoudre vos projets. Mais vous êtes ici pour apprendre, pas pour prouver que l'IA a appris. Ne perdez pas votre temps (ni le nôtre) simplement pour démontrer que l'IA peut résoudre le problème posé.
- Apprendre à 42 ne consiste pas à connaître la réponse — il s'agit de développer la capacité à en trouver une. L'IA donne la réponse directement, mais cela vous empêche de construire votre propre raisonnement. Or le raisonnement prend du temps, de l'effort et implique l'échec. Le chemin du succès n'est pas censé être facile.
- Gardez en tête que pendant les examens, l'IA n'est pas disponible — pas d'internet, pas de smartphones, etc. Vous réaliserez vite si vous avez trop compté sur l'IA dans votre apprentissage.
- L'apprentissage par les pairs vous expose à des idées et des approches différentes, en améliorant vos compétences interpersonnelles et votre capacité à penser de manière divergente. C'est bien plus précieux que de discuter avec un bot. Alors ne soyez pas timides — parlez, posez des questions et apprenez ensemble !
- Oui, l'IA fera partie du cursus — à la fois comme outil d'apprentissage et comme sujet en soi. Vous aurez même la chance de construire votre propre logiciel d'IA. Pour en savoir plus sur notre approche crescendo, consultez la documentation disponible sur l'intranet.

**✓ Bonne pratique :** Je bloque sur un nouveau concept. Je demande à quelqu'un à côté comment il a abordé le problème. Nous parlons 10 minutes — et soudain, c'est clair. J'ai compris.

**✗ Mauvaise pratique :** J'utilise l'IA en secret, je copie du code qui a l'air correct. Pendant l'évaluation par les pairs, je suis incapable d'expliquer quoi que ce soit. J'échoue. Pendant l'examen — pas d'IA — je suis à nouveau bloqué. J'échoue.

---

## Chapitre IV — Règles générales

- L'utilisation de **VirtualBox** (ou **UTM** si vous ne pouvez pas utiliser VirtualBox) est **obligatoire**.
- Vous ne devez rendre qu'un fichier `signature.txt` à la racine de votre dépôt. Vous devez y coller la **signature du disque virtuel** de votre machine. Voir la section « Soumission et évaluation par les pairs » pour plus d'informations.
- **Aucun snapshot ne doit exister au début de chaque évaluation.**

---

## Chapitre V — Partie obligatoire

Ce projet consiste à mettre en place votre premier serveur en suivant des règles précises.

> Comme il s'agit de mettre en place un serveur, vous n'installerez qu'un ensemble minimal de services. Pour cette raison, une interface graphique est inutile ici. Il est donc **interdit d'installer X.org, Wayland ou tout autre serveur graphique équivalent. Sinon, votre note sera de 0.**

Vous devez choisir comme système d'exploitation soit la **dernière version stable de Debian** (pas de testing/unstable), soit la **dernière version stable de Rocky**. Debian est fortement recommandé si vous débutez en administration système.

> La mise en place de Rocky est assez complexe. Vous n'avez donc pas à configurer KDump. En revanche, **SELinux** doit être actif au démarrage et sa configuration doit être adaptée aux besoins du projet. **AppArmor** pour Debian doit également être actif au démarrage.

Vous devez créer **au moins 2 partitions chiffrées avec LVM**. Voici un exemple de partitionnement possible :

```text
# lsblk
NAME                        MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINT
sda                           8:0    0  30.8G  0 disk
├─sda1                        8:1    0   500M  0 part  /boot
├─sda2                        8:2    0     1K  0 part
└─sda5                        8:5    0  30.3G  0 part
  └─sda5_crypt              254:0    0  30.3G  0 crypt
    ├─LVMGroup-root         254:1    0    10G  0 lvm   /
    ├─LVMGroup-swap         254:2    0   2.3G  0 lvm   [SWAP]
    ├─LVMGroup-home         254:3    0     5G  0 lvm   /home
    ├─LVMGroup-var          254:4    0     3G  0 lvm   /var
    ├─LVMGroup-srv          254:5    0     3G  0 lvm   /srv
    ├─LVMGroup-tmp          254:6    0     3G  0 lvm   /tmp
    └─LVMGroup-var--log     254:7    0     4G  0 lvm   /var/log
sr0                          11:0    1  1024M  0 rom
```

> Pendant la soutenance, quelques questions vous seront posées sur le système d'exploitation choisi. Par exemple, vous devez connaître les différences entre `aptitude` et `apt`, ou savoir ce qu'est SELinux ou AppArmor. En bref, comprenez ce que vous utilisez !

Un **service SSH doit tourner sur le port 4242** dans votre machine virtuelle. Pour des raisons de sécurité, il ne doit **pas** être possible de se connecter en SSH en tant que **root**.

> L'utilisation de SSH sera testée pendant la soutenance en créant un nouveau compte. Vous devez donc comprendre son fonctionnement.

Vous devez configurer votre système d'exploitation avec le pare-feu **UFW** (ou **firewalld** pour Rocky) et ne laisser **que le port 4242 ouvert** dans votre machine virtuelle.

> L'exemple montre des tailles de disque arbitraires. Vous devez déterminer la taille appropriée de chaque partition pour assurer un fonctionnement correct tout en évitant une utilisation inutile du disque.

Votre pare-feu doit être **actif** au lancement de votre machine virtuelle.

> Pour Rocky, vous devez utiliser `firewalld` au lieu d'UFW.

- Le **nom d'hôte** de votre machine virtuelle doit être **votre login suivi de 42** (par exemple `wil42`).
  - Vous devrez **modifier ce nom d'hôte pendant votre évaluation par les pairs**.
- Vous devez mettre en place une **politique de mots de passe robuste**.
- Vous devez installer et configurer `sudo` selon des règles strictes.
- En plus de l'utilisateur `root`, un utilisateur **ayant votre login comme nom d'utilisateur** doit être présent.
- Cet utilisateur doit appartenir aux groupes **`user42`** et **`sudo`**.

> Pendant votre évaluation, vous devrez **créer un nouvel utilisateur** et l'assigner à un groupe.

Pour mettre en place une politique de mots de passe robuste, vous devez respecter les exigences suivantes :

- Votre mot de passe doit **expirer tous les 30 jours**.
- Le **nombre minimum de jours entre deux changements** de mot de passe doit être fixé à **2**.
- L'utilisateur doit recevoir un **message d'avertissement 7 jours** avant l'expiration de son mot de passe.
- Votre mot de passe doit comporter **au moins 10 caractères**. Il doit contenir une **majuscule**, une **minuscule** et un **chiffre**. Il ne doit pas contenir **plus de 3 caractères identiques consécutifs**.
- Le mot de passe ne doit **pas contenir le nom de l'utilisateur**.
- La règle suivante **ne s'applique pas** au mot de passe de `root` : le mot de passe doit contenir **au moins 7 caractères qui ne faisaient pas partie du mot de passe précédent**.
- Bien entendu, **le mot de passe de `root` doit respecter cette politique**.

> Après avoir configuré vos fichiers, vous devrez **changer tous les mots de passe** des comptes présents sur la machine virtuelle, y compris le compte `root`.

Pour configurer solidement votre groupe `sudo`, vous devez respecter les exigences suivantes :

- L'authentification via `sudo` doit être **limitée à 3 tentatives** en cas de mot de passe incorrect.
- Un **message personnalisé** de votre choix doit être affiché si un mot de passe incorrect est saisi lors de l'utilisation de `sudo`.
- **Chaque action effectuée avec `sudo` doit être journalisée**, **entrées et sorties** comprises. Le fichier de log doit être enregistré dans le dossier `/var/log/sudo/`.
- Le **mode TTY** doit être activé pour des raisons de sécurité.
- Pour des raisons de sécurité, les **chemins utilisables par `sudo`** doivent également être restreints.

Exemple :

```text
/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin
```

Enfin, vous devez créer un script simple nommé **`monitoring.sh`**. Il doit être **écrit en bash**.

Au démarrage du serveur, le script affichera les informations listées ci-dessous sur tous les terminaux, et toutes les **10 minutes** (jetez un œil à `wall`). La bannière est optionnelle. **Aucune erreur ne doit être affichée.**

Votre script doit toujours pouvoir afficher les informations suivantes :

- L'**architecture** de votre système d'exploitation et sa **version du noyau**.
- Le nombre de **processeurs physiques**.
- Le nombre de **processeurs virtuels**.
- La **RAM disponible** sur votre serveur et son **taux d'utilisation en pourcentage**.
- Le **stockage disponible** sur votre serveur et son **taux d'utilisation en pourcentage**.
- Le **taux d'utilisation CPU** actuel en pourcentage.
- La **date et l'heure du dernier redémarrage**.
- L'activation ou non de **LVM**.
- Le nombre de **connexions actives**.
- Le nombre d'**utilisateurs utilisant le serveur**.
- L'**adresse IPv4** de votre serveur et son **adresse MAC**.
- Le nombre de **commandes exécutées avec `sudo`**.

> Pendant votre évaluation, il vous sera demandé d'expliquer le fonctionnement de ce script. Vous devrez également l'**interrompre sans le modifier**.

> Jetez un œil à `cron`.

Voici un exemple du fonctionnement attendu du script :

```text
Broadcast message from root@wil (tty1) (Sun Apr 25 15:45:00 2021):
#Architecture: Linux wil 4.19.0-16-amd64 #1 SMP Debian 4.19.181-1 (2021-03-19) x86_64 GNU/Linux
#Physical CPU: 1
#vCPU: 1
#Memory Usage: 74/987MB (7.50%)
#Disk Usage: 1009/2Gb (49%)
#CPU load: 6.7%
#Last boot: 2021-04-25 14:45
#LVM use: yes
#TCP Connections: 1 ESTABLISHED
#User log: 1
#Network: IP 10.0.2.15 (08:00:27:51:9b:a5)
#Sudo: 42 cmd
```

Voici deux commandes utiles pour vérifier certaines exigences du sujet :

**Pour Rocky :**

```console
root@wil:~# head -n 2 /etc/os-release
NAME="Rocky Linux"
VERSION="8.7 (Green Obsidian)"
root@wil:~# sestatus
SELinux status:                 enabled
SELinuxfs mount:                /sys/fs/selinux
SELinux root directory:         /etc/selinux
Loaded policy name:             targeted
Current mode:                   enforcing
Mode from config file:          enforcing
Policy MLS status:              enabled
Policy deny_unknown status:     allowed
Memory protection checking:     actual (secure)
Max kernel policy version:      33
root@wil:~# ss -tunlp
Netid State  Recv-Q Send-Q Local Address:Port Peer Address:Port Process
tcp   LISTEN 0      128          0.0.0.0:4242      0.0.0.0:*     users:(("sshd",pid=20429,fd=6))
tcp   LISTEN 0      128             [::]:4242         [::]:*     users:(("sshd",pid=20429,fd=4))
root@wil:~# firewall-cmd --list-services
ssh
root@wil:~# firewall-cmd --list-port
4242/tcp
root@wil:~# firewall-cmd --state
running
root@wil:~#
```

**Pour Debian :**

```console
root@wil:~# head -n 2 /etc/os-release
PRETTY_NAME="Debian GNU/Linux 10 (buster)"
NAME="Debian GNU/Linux"
root@wil:~# /usr/sbin/aa-status
apparmor module is loaded.
root@wil:~# ss -tunlp
Netid State  Recv-Q Send-Q Local Address:Port Peer Address:Port
tcp   LISTEN 0      128          0.0.0.0:4242      0.0.0.0:*     users:(("sshd",pid=523,fd=3))
tcp   LISTEN 0      128             [::]:4242         [::]:*     users:(("sshd",pid=523,fd=4))
root@wil:~# /usr/sbin/ufw status
Status: active

To                         Action      From
--                         ------      ----
4242                       ALLOW       Anywhere
4242 (v6)                  ALLOW       Anywhere (v6)
```

---

## Chapitre VI — Exigences du README

Un fichier `README.md` doit être fourni à la racine de votre dépôt Git. Son but est de permettre à toute personne non familière avec le projet (pairs, staff, recruteurs, etc.) de comprendre rapidement de quoi il s'agit, comment l'exécuter et où trouver plus d'informations.

Le `README.md` doit contenir au minimum :

- La toute première ligne doit être **en italique** et lire : `*This project has been created as part of the 42 curriculum by <login1>[, <login2>[, <login3>[...]]].*`
- Une section « **Description** » présentant clairement le projet, avec son **but** et un aperçu bref.
- Une section « **Instructions** » contenant toute information utile sur la **compilation, l'installation et/ou l'exécution**.
- Une section « **Resources** » listant les références classiques liées au sujet (documentation, articles, tutoriels, etc.), ainsi qu'une **description de l'usage de l'IA** — en précisant pour quelles tâches et quelles parties du projet.

➠ Des sections supplémentaires peuvent être exigées selon le projet (par exemple exemples d'utilisation, liste des fonctionnalités, choix techniques, etc.).

Toute addition requise sera explicitement listée ci-dessous.

- Une section **Project description** doit également expliquer votre **choix de système d'exploitation** (Debian ou Rocky), avec leurs **avantages et inconvénients** respectifs. Elle doit décrire les **principaux choix de conception** faits lors de l'installation (partitionnement, politiques de sécurité, gestion des utilisateurs, services installés) et fournir une comparaison entre :
  - **Debian vs Rocky Linux**
  - **AppArmor vs SELinux**
  - **UFW vs firewalld**
  - **VirtualBox vs UTM**

> L'anglais est recommandé ; vous pouvez sinon utiliser la langue principale de votre campus.

---

## Chapitre VII — Partie bonus

Fonctionnalités bonus :

- Configurer les **partitions correctement** afin d'obtenir une structure similaire à celle ci-dessous :

```text
# lsblk
NAME                        MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINT
sda                           8:0    0  30.8G  0 disk
├─sda1                        8:1    0   500M  0 part  /boot
├─sda2                        8:2    0     1K  0 part
└─sda5                        8:5    0  30.3G  0 part
  └─sda5_crypt              254:0    0  30.3G  0 crypt
    ├─LVMGroup-root         254:1    0    10G  0 lvm   /
    ├─LVMGroup-swap         254:2    0   2.3G  0 lvm   [SWAP]
    ├─LVMGroup-home         254:3    0     5G  0 lvm   /home
    ├─LVMGroup-var          254:4    0     3G  0 lvm   /var
    ├─LVMGroup-srv          254:5    0     3G  0 lvm   /srv
    ├─LVMGroup-tmp          254:6    0     3G  0 lvm   /tmp
    └─LVMGroup-var--log     254:7    0     4G  0 lvm   /var/log
sr0                          11:0    1  1024M  0 rom
```

- Mettre en place un **site WordPress fonctionnel** avec les services suivants : **lighttpd**, **MariaDB** et **PHP**.
- Mettre en place un **service de votre choix** que vous jugez utile (**NGINX / Apache2 exclus !**). Pendant la soutenance, vous devrez **justifier votre choix**.

> L'exemple montre des tailles de disque arbitraires. Vous devez déterminer la taille appropriée de chaque partition pour assurer un fonctionnement correct tout en évitant une utilisation inutile du disque.

> Pour compléter la partie bonus, vous avez la possibilité de mettre en place des services supplémentaires. Dans ce cas, **vous pouvez ouvrir davantage de ports** selon vos besoins. Bien entendu, les **règles UFW/firewalld doivent être adaptées** en conséquence.

> La partie bonus ne sera évaluée que si la **partie obligatoire est PARFAITE**. Parfait signifie que la partie obligatoire a été entièrement réalisée et fonctionne sans aucun dysfonctionnement. Si vous n'avez pas validé TOUTES les exigences obligatoires, votre partie bonus ne sera pas évaluée du tout.

---

## Chapitre VIII — Soumission et évaluation par les pairs

Vous ne devez rendre que le `README.md` attendu et un fichier `signature.txt` à la racine de votre dépôt Git. Vous devez coller la signature du disque virtuel de votre machine dans ce fichier. Pour obtenir cette signature, ouvrez d'abord le dossier d'installation par défaut (le dossier où vos VM sont enregistrées) :

- **Windows** : `%HOMEDRIVE%%HOMEPATH%\VirtualBox VMs\`
- **Linux** : `~/VirtualBox VMs/`
- **MacM1** : `~/Library/Containers/com.utmapp.UTM/Data/Documents/`
- **MacOS** : `~/VirtualBox VMs/`

Récupérez ensuite la signature du fichier `.vdi` (ou `.qcow2` pour les utilisateurs d'UTM) de votre machine virtuelle au format **sha1**. Voici quatre exemples de commandes pour un fichier `rocky_serv.vdi` :

- **Windows** : `certUtil -hashfile rocky_serv.vdi sha1`
- **Linux** : `sha1sum rocky_serv.vdi`
- **Mac M1** : `shasum rocky.utm/Images/disk-0.qcow2`
- **MacOS** : `shasum rocky_serv.vdi`

Voici un exemple de sortie :

```text
6e657c4619944be17df3c31faa030c25e43e40af
```

> Veuillez noter que la signature de votre machine virtuelle sera **modifiée dès que vous redémarrerez la machine**. Pour permettre la vérification de la signature lors de toutes vos évaluations, vous pouvez soit dupliquer le fichier disque de la machine virtuelle, soit utiliser des snapshots pour chaque évaluation.
>
> Il est bien entendu **INTERDIT d'inclure votre machine virtuelle dans votre dépôt Git**. Pendant la soutenance, la signature du fichier `signature.txt` sera comparée à celle de votre machine virtuelle. Si les deux ne sont pas identiques, **votre note sera de 0**.

> L'usage des snapshots est restreint. Chaque soutenance doit commencer **sans aucun snapshot**. Un snapshot dédié à la soutenance sera ensuite créé puis supprimé à la fin de celle-ci. Vous êtes encouragés à tester la fonctionnalité de snapshots avant de rendre le projet.

Pendant l'évaluation, une brève **modification du projet** peut parfois être demandée. Cela peut impliquer un changement mineur de comportement, quelques lignes de code à écrire ou réécrire, ou une fonctionnalité facile à ajouter.

Bien que cette étape ne s'applique pas nécessairement à tous les projets, vous devez y être préparé si elle est mentionnée dans les consignes d'évaluation.

Cette étape vise à vérifier votre compréhension réelle d'une partie précise du projet. La modification peut être effectuée dans l'environnement de développement de votre choix (par exemple votre configuration habituelle), et doit être réalisable en quelques minutes — sauf si un délai spécifique est défini dans les consignes.

On peut par exemple vous demander une petite mise à jour d'une fonction ou d'un script, une modification d'affichage, ou l'ajustement d'une structure de données pour stocker une nouvelle information, etc.

Les détails (portée, cible, etc.) seront précisés dans les consignes d'évaluation et peuvent varier d'une évaluation à l'autre pour le même projet.
