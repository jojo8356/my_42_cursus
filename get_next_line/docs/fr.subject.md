# Get Next Line — Lire une ligne depuis un descripteur de fichier est bien trop fastidieux

> **Sujet officiel du cursus 42** — **Version 14.3** (Nouveau Tronc Commun 2025–2026)  
> **Résumé :** *Ce projet a pour objectif de vous faire programmer une fonction qui renvoie une ligne lue depuis un descripteur de fichier (`file descriptor`).*

---

## Table des matières

- [Chapitre I — Objectifs](#chapitre-i--objectifs)
- [Chapitre II — Consignes générales](#chapitre-ii--consignes-générales)
- [Chapitre III — Directives relatives à l'IA](#chapitre-iii--directives-relatives-à-lia)
- [Chapitre IV — Partie obligatoire](#chapitre-iv--partie-obligatoire)
- [Chapitre V — Exigences du fichier README.md](#chapitre-v--exigences-du-fichier-readmemd)
- [Chapitre VI — Partie bonus](#chapitre-vi--partie-bonus)
- [Chapitre VII — Rendu et peer-évaluation](#chapitre-vii--rendu-et-peer-évaluation)

---

## Chapitre I — Objectifs

Ce projet vous permettra non seulement d'ajouter une fonction particulièrement pratique à votre boîte à outils personnelle, mais il vous fera également découvrir un concept fondamental de la programmation en langage C : **les variables statiques** (`static`).

---

## Chapitre II — Consignes générales

- Votre projet doit être écrit en **C**.
- Votre projet doit respecter la **Norme** (Norme v4.1 de 42). Si vous avez des fichiers ou fonctions bonus, ils sont inclus dans la vérification de la Norme et vous obtiendrez **0** en cas d'erreur de norme.
- Vos fonctions ne doivent pas s'arrêter de manière inattendue (*segmentation fault*, *bus error*, *double free*, etc.), en dehors des cas explicites de comportement indéfini (*undefined behavior*). Si cela se produit, votre projet sera considéré comme non fonctionnel et recevra la note de **0** lors de l'évaluation.
- Toute mémoire allouée sur le tas (*heap*) doit être correctement libérée lorsque nécessaire. **Aucune fuite mémoire (*memory leak*) ne sera tolérée.**
- Si le sujet l'exige, vous devez rendre un `Makefile` compilant vos sources avec les drapeaux `-Wall -Wextra -Werror` via `cc`, sans effectuer de *relink* inutile. *(Note : `get_next_line` ne demande pas de `Makefile` de rendu ; la Moulinette compile directement les sources à la racine).*
- Les bonus doivent être placés dans des fichiers suffixés par `_bonus.{c/h}`. L'évaluation de la partie obligatoire et celle de la partie bonus sont effectuées séparément.
- Nous vous encourageons vivement à créer des programmes de test pour votre projet (bien qu'ils **n'aient pas à être rendus et ne soient pas notés**). Ils vous seront précieux pour vérifier votre travail et celui de vos pairs en soutenance.
- Rendez votre travail sur le dépôt Git qui vous est assigné. Seul le contenu présent dans ce dépôt sera évalué. Si *Deepthought* (la Moulinette) corrige votre travail après vos peer-évaluations, l'évaluation s'arrêtera à la moindre erreur rencontrée.

---

## Chapitre III — Directives relatives à l'IA

### Contexte
Ce projet vise à ancrer les briques fondamentales de votre formation à 42. Un apprentissage solide exige un véritable effort intellectuel personnel par l'expérimentation, la répétition et l'échange entre pairs (*peer-learning*).

### Règles pour l'apprenant
- Appliquez votre propre raisonnement avant de solliciter un outil d'IA.
- Ne demandez pas directement la solution clé en main à une IA : lors des examens (sans réseau ni IA) et lors des soutenances, vous devez maîtriser et savoir réécrire ou adapter chaque ligne de votre code.
- Documentez de façon transparente tout usage éventuel de l'IA dans la section `Resources` de votre `README.md`.

---

## Chapitre IV — Partie obligatoire

| Champ | Spécification |
| :--- | :--- |
| **Nom de la fonction** | `get_next_line` |
| **Prototype** | `char *get_next_line(int fd);` |
| **Fichiers de rendu** | `get_next_line.c`, `get_next_line_utils.c`, `get_next_line.h`, **tous à la racine du dépôt** |
| **Paramètres** | `fd` : le descripteur de fichier depuis lequel lire |
| **Valeur de retour** | **Ligne lue** : comportement correct<br>**`NULL`** : il n'y a plus rien à lire, ou une erreur est survenue |
| **Fonctions externes autorisées** | `read`, `malloc`, `free` |
| **Description** | Écrire une fonction qui retourne une ligne lue depuis un descripteur de fichier |

### Spécifications détaillées

- Des appels successifs (par exemple dans une boucle) à votre fonction `get_next_line()` doivent permettre de lire le fichier texte pointé par le descripteur de fichier, **une ligne à la fois**.
- Votre fonction doit retourner la ligne qui vient d'être lue. S'il n'y a plus rien à lire ou si une erreur survient, elle doit retourner `NULL`.
- Assurez-vous que votre fonction se comporte correctement aussi bien lors de la lecture d'un **fichier** que lors de la lecture sur **l'entrée standard (`stdin`)**.
- **Important :** la ligne retournée doit **inclure le caractère `\n` final**, sauf si la fin de fichier (*EOF*) a été atteinte et que le fichier ne se termine pas par un `\n`.
- Le fichier d'en-tête `get_next_line.h` doit contenir au minimum le prototype de la fonction `get_next_line()`.
- Ajoutez toutes les fonctions utilitaires nécessaires dans le fichier `get_next_line_utils.c`.

> *Un bon point de départ consiste à comprendre ce qu'est une variable statique (`static`).*

- Comme vous devrez appeler `read()` dans `get_next_line()`, le compilateur sera invoqué avec l'option `-D BUFFER_SIZE=n`, qui définit la taille du tampon passée à `read()`. Cette valeur sera modifiée par vos correcteurs et par la Moulinette pour tester votre code.

> **Le projet doit impérativement pouvoir compiler AVEC et SANS le drapeau `-D BUFFER_SIZE` en plus des drapeaux habituels (`-Wall -Wextra -Werror`). Vous êtes libre de choisir la valeur par défaut qui vous convient.**

- Votre code sera compilé de la manière suivante (exemple avec un tampon de taille `42`) :
  ```bash
  cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 <fichiers>.c
  ```
  *(Note : les fichiers doivent obligatoirement se trouver à la racine du dépôt).*
- `get_next_line()` a un comportement indéfini si le fichier pointé par `fd` a été modifié depuis le dernier appel alors que `read()` n'avait pas encore atteint la fin du fichier.
- `get_next_line()` a également un comportement indéfini lors de la lecture d'un fichier binaire. Cependant, vous pouvez implémenter une manière logique et cohérente de gérer ce cas si vous le souhaitez.

> *Votre fonction fonctionne-t-elle toujours si `BUFFER_SIZE` vaut `9999` ? Et s'il vaut `1` ? Ou `10000000` ? Savez-vous pourquoi ?*

> **Essayez de lire le moins possible à chaque appel de `get_next_line()`. Dès que vous rencontrez un retour à la ligne (`\n`), vous devez retourner la ligne courante. Ne lisez surtout pas l'intégralité du fichier d'un coup avant de le découper.**

### Interdictions strictes
- Vous **n'êtes pas autorisé** à utiliser votre `libft` dans ce projet.
- L'appel système `lseek()` est **interdit**.
- Les **variables globales** sont **interdites**.

---

## Chapitre V — Exigences du fichier `README.md`

Un fichier `README.md` doit être fourni à la **racine de votre dépôt Git**. Il doit permettre à toute personne découvrant le projet (pairs, équipe pédagogique, recruteurs) de comprendre rapidement son objectif, comment le compiler/l'exécuter et où trouver des ressources.

Le `README.md` doit obligatoirement inclure :
- **Toute première ligne** en italique au format exact :  
  `*This project has been created as part of the 42 curriculum by <login1>[, <login2>[, ...]].*`
- Une section **`Description`** présentant clairement le projet, son but et une vue d'ensemble.
- Une section **`Instructions`** détaillant la compilation, l'installation et/ou l'exécution.
- Une section **`Resources`** listant les références classiques (pages `man`, articles, documentation) ainsi qu'une description précise de l'utilisation éventuelle de l'IA (tâches et parties concernées).
- **Exigence spécifique à `get_next_line` :**  
  Une **explication détaillée et une justification de l'algorithme choisi** pour ce projet doivent impérativement figurer dans le `README.md`.

---

## Chapitre VI — Partie bonus

Ce projet est direct et ne se prête pas à des bonus extravagants. Toutefois, si votre partie obligatoire est **parfaite**, voici les exigences de la partie bonus :

1. **Développer `get_next_line()` en utilisant UNE SEULE variable statique (`static`).**
2. **Gérer plusieurs descripteurs de fichiers (`fd`) simultanément.**  
   Par exemple, si les descripteurs `3`, `4` et `5` sont ouverts en lecture, vous devez pouvoir appeler `get_next_line()` sur `fd 3`, puis sur `fd 4`, puis sur `fd 5`, puis à nouveau sur `fd 3`, puis `fd 4`, etc., **sans jamais perdre le fil de lecture de chaque descripteur ni mélanger le contenu des différents flux**.

Ajoutez le suffixe `_bonus.[c/h]` aux fichiers de la partie bonus. En complément des 3 fichiers obligatoires, vous rendrez donc les **3 fichiers suivants à la racine du dépôt** :
- `get_next_line_bonus.c`
- `get_next_line_bonus.h`
- `get_next_line_utils_bonus.c`

> *La partie bonus ne sera évaluée que si la partie obligatoire est PARFAITE (100 % fonctionnelle, sans fuite mémoire ni erreur de Norme).*

---

## Chapitre VII — Rendu et peer-évaluation

Rendez votre travail dans votre dépôt Git habituel et vérifiez scrupuleusement le nom exact de chacun des 6 fichiers (`get_next_line.{c,h}`, `get_next_line_utils.c`, `get_next_line_bonus.{c,h}`, `get_next_line_utils_bonus.c`) ainsi que `README.md`.

Lors de la préparation de vos tests, gardez à l'esprit que :
1. La taille du tampon (`BUFFER_SIZE`) comme la longueur des lignes peuvent varier de façon extrême (`1` octet à plusieurs mégaoctets).
2. Un descripteur de fichier (`fd`) ne pointe pas uniquement vers un fichier régulier (entrée standard `0`, tubes `pipe`, sockets, FIFOs, etc.).

Une fois le projet validé, n'hésitez pas à intégrer `get_next_line()` à votre `libft` pour vos prochains projets du cursus.
