# Pipex — sujet français (v2)

*Transcription de `docs/fr.subject.pdf`. Le PDF fait foi ; ce fichier
existe pour pouvoir lire, chercher et comparer le sujet sans lecteur
PDF. La numérotation des chapitres suit l'original.*

---

## Chapitre I — Avant-propos

> Cristina : « Go dance salsa somewhere :) »

## Chapitre II — Instructions communes

- Votre projet doit être écrit en C.
- Votre projet doit être écrit conformément à la Norme. Si vous avez
  des fichiers/fonctions bonus, ils sont inclus dans la vérification
  de la norme et vous recevrez un 0 s'il y a une erreur de norme à
  l'intérieur.
- Vos fonctions ne doivent pas quitter de manière inattendue (erreur de
  segmentation, erreur de bus, double free, etc.) en dehors des
  comportements indéfinis. Si cela arrive, votre projet sera considéré
  comme non fonctionnel et recevra un 0 lors de l'évaluation.
- Tout l'espace mémoire alloué sur le tas doit être correctement libéré
  lorsque c'est nécessaire. Aucune fuite ne sera tolérée.
- Si le sujet le demande, vous devez rendre un `Makefile` qui compilera
  vos fichiers sources vers la sortie requise avec les flags `-Wall`,
  `-Wextra` et `-Werror`, en utilisant `cc`, et votre Makefile ne doit
  pas relink.
- Votre Makefile doit au minimum contenir les règles `$(NAME)`, `all`,
  `clean`, `fclean` et `re`.
- Pour rendre des bonus à votre projet, vous devez inclure une règle
  `bonus` à votre Makefile, qui ajoutera tous les headers, librairies
  ou fonctions interdits sur la partie principale du projet. Les bonus
  doivent être dans un fichier différent `_bonus.{c/h}` si le sujet ne
  spécifie rien d'autre. L'évaluation de la partie obligatoire et de la
  partie bonus est faite séparément.
- Si votre projet vous autorise à utiliser votre libft, vous devez
  copier ses sources et son Makefile associé dans un dossier `libft`
  avec son Makefile associé. Le Makefile de votre projet doit compiler
  la librairie en utilisant son Makefile, puis compiler le projet.
- Nous vous encourageons à créer des programmes de test pour votre
  projet, même si ce travail n'aura pas à être rendu et ne sera pas
  noté. Cela vous donnera l'occasion de tester facilement votre travail
  et celui de vos pairs. Vous trouverez ces tests particulièrement
  utiles lors de votre soutenance. En effet, pendant la soutenance,
  vous êtes libre d'utiliser vos tests et/ou les tests du pair que vous
  évaluez.
- Rendez votre travail sur votre dépôt git assigné. Seul le travail
  dans le dépôt git sera noté. Si Deepthought est assigné pour noter
  votre travail, cela sera fait après vos évaluations par les pairs. Si
  une erreur se produit dans une section de votre travail pendant la
  notation de Deepthought, l'évaluation s'arrêtera.

## Chapitre III — Partie obligatoire

Ce projet porte sur la gestion des pipes.

Votre programme sera exécuté de la façon suivante :

```sh
./pipex file1 cmd1 cmd2 file2
```

Il doit prendre 4 arguments :

- `file1` et `file2` sont des noms de fichiers.
- `cmd1` et `cmd2` sont des commandes shell avec leurs paramètres.

Il doit se comporter exactement comme la commande shell ci-dessous :

```sh
$> < file1 cmd1 | cmd2 > file2
```

### Tableau récapitulatif

| | |
| :--- | :--- |
| **Nom du programme** | `pipex` |
| **Fichiers à rendre** | `Makefile`, `*.h`, `*.c` |
| **Makefile** | `NAME`, `all`, `clean`, `fclean`, `re` |
| **Arguments** | `file1 cmd1 cmd2 file2` |
| **Fonctions externes** | `open`, `close`, `read`, `write`, `malloc`, `free`, `perror`, `strerror`, `access`, `dup`, `dup2`, `execve`, `exit`, `fork`, `pipe`, `unlink`, `wait`, `waitpid` |
| **Libft autorisée** | Oui |
| **Description** | Ce projet porte sur la gestion des pipes. |

### III.1 Exemples

```sh
$> ./pipex infile "ls -l" "wc -l" outfile
```
Devrait être identique à :
```sh
< infile ls -l | wc -l > outfile
```

```sh
$> ./pipex infile "grep a1" "wc -w" outfile
```
Devrait être identique à :
```sh
< infile grep a1 | wc -w > outfile
```

### III.2 Prérequis

Votre projet doit respecter les règles suivantes :

- Vous devez rendre un Makefile qui compilera vos fichiers sources. Il
  ne doit pas relink.
- Vous devez gérer les erreurs avec du bon sens. En aucun cas votre
  programme ne doit quitter de manière inattendue (erreur de
  segmentation, erreur de bus, double free, etc.).
- Votre programme ne doit pas avoir de fuites de mémoire.
- Si vous avez le moindre doute, référez-vous à la commande shell :
  ```sh
  < file1 cmd1 | cmd2 > file2
  ```

## Chapitre IV — Partie bonus

> Les bonus ne seront évalués que si la partie obligatoire est
> **PARFAITE**. Par parfaite, nous entendons complète et sans aucun
> dysfonctionnement. Si vous n'avez pas réussi TOUS les points de la
> partie obligatoire, votre partie bonus ne sera pas prise en compte.

Vous aurez des points supplémentaires si vous :

- **Gérez plusieurs pipes.**

  Ceci :
  ```sh
  $> ./pipex file1 cmd1 cmd2 cmd3 ... cmdn file2
  ```
  Devrait être identique à :
  ```sh
  < file1 cmd1 | cmd2 | cmd3 ... | cmdn > file2
  ```

- **Gérez `<<` et `>>` quand le premier paramètre est "here_doc".**

  Ceci :
  ```sh
  $> ./pipex here_doc LIMITER cmd cmd1 file
  ```
  Devrait être identique à :
  ```sh
  cmd << LIMITER | cmd1 >> file
  ```

## Chapitre V — Rendu et évaluation par les pairs

Rendez votre travail sur votre dépôt Git comme d'habitude. Seul le
travail à l'intérieur de votre dépôt sera évalué pendant la soutenance.
N'hésitez pas à vérifier les noms de vos fichiers pour vous assurer
qu'ils sont corrects.
