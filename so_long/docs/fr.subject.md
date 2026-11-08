# so_long — sujet (transcription française)

> Traduction française de `docs/en.subject.pdf`. Le PDF anglais fait foi ;
> cette version existe pour vérifier la compréhension des exigences, pas pour
> les remplacer. La numérotation des chapitres, les contraintes et les cartes
> d'exemple suivent l'original.

**So Long, And Thanks for All the Fish !**

**Résumé :** ce projet est un très petit jeu 2D. Il est conçu pour vous faire
travailler avec des textures et des sprites, ainsi qu'avec quelques éléments de
gameplay très basiques.

## Sommaire

- I. Avant-propos — 2
- II. Objectifs — 3
- III. Consignes communes — 4
- IV. Partie obligatoire — 5
- V. Partie bonus — 8
- VI. Exemples — 9

---

## Chapitre I — Avant-propos

Être développeur est une bonne chose pour créer son propre jeu. Mais un bon jeu
a besoin de bons assets.

Pour les jeux 2D, cherchez des tiles, des tilesets, des sprites et des sprite
sheets. Heureusement, certains artistes talentueux partagent leur travail sur des
plateformes comme itch.io.

Quoi que vous fassiez, respectez le travail des autres.

---

## Chapitre II — Objectifs

Les objectifs de ce projet sont similaires à tous ceux de cette première année :
rigueur, utilisation du C, utilisation d'algorithmes de base, recherche
d'information, etc.

En tant que projet de design graphique, `so_long` vous permettra d'améliorer vos
compétences dans ces domaines : fenêtres, couleurs, événements, textures, etc.

---

## Chapitre III — Consignes communes

- Votre projet doit être écrit en accord avec la Norme. Si vous avez des
  fichiers/fonctions bonus, ils sont inclus dans le contrôle de norme et vous
  recevrez un 0 s'il y a une erreur de norme à l'intérieur.
- Vos fonctions ne doivent pas quitter de façon inattendue (segmentation fault,
  bus error, double free, etc.) hormis les comportements indéfinis. Si cela
  arrive, votre projet sera considéré non fonctionnel et recevra un 0 pendant
  l'évaluation.
- Tout l'espace mémoire alloué sur le tas doit être correctement libéré quand
  c'est nécessaire. Aucune fuite ne sera tolérée.
- Si le sujet l'exige, vous devez rendre un `Makefile` qui compile vos fichiers
  sources vers la sortie attendue avec les flags `-Wall`, `-Wextra` et
  `-Werror`, et votre `Makefile` ne doit pas relinker.
- Votre `Makefile` doit au moins contenir les règles `$(NAME)`, `all`, `clean`,
  `fclean` et `re`.
- Pour rendre des bonus, vous devez ajouter une règle `bonus` à votre
  `Makefile`, qui ajoutera les différents en-têtes, bibliothèques ou fonctions
  interdits dans la partie principale. Les bonus doivent être dans des fichiers
  différents `_bonus.{c/h}`. L'évaluation de la partie obligatoire et celle des
  bonus sont faites séparément.
- Si votre projet vous autorise à utiliser votre `libft`, vous devez copier ses
  sources et son `Makefile` associé dans un dossier `libft` avec son `Makefile`.
  Le `Makefile` de votre projet doit compiler la bibliothèque en utilisant son
  `Makefile`, puis compiler le projet.
- Nous vous encourageons à créer des programmes de test pour votre projet, même
  si ce travail n'aura pas à être rendu et ne sera pas noté. Cela vous donnera
  l'occasion de tester facilement votre travail et celui de vos camarades. Vous
  trouverez ces tests particulièrement utiles lors de votre soutenance. En effet,
  pendant la soutenance, vous êtes libres d'utiliser vos tests et/ou ceux du
  camarade que vous évaluez.
- Rendez votre travail dans le dépôt git qui vous est assigné. Seul le travail
  présent dans le dépôt git sera noté. Si Deepthought est assigné à la notation
  de votre travail, elle sera faite après vos évaluations par les pairs. Si une
  erreur survient dans n'importe quelle section de votre travail pendant la
  notation de Deepthought, l'évaluation s'arrête.

---

## Chapitre IV — Partie obligatoire

| | |
|---|---|
| **Nom du programme** | `so_long` |
| **Fichiers à rendre** | Tous vos fichiers |
| **Makefile** | `all`, `clean`, `fclean`, `re`, `bonus` |
| **Arguments** | une carte au format `*.ber` |
| **Fonctions externes** | `open`, `close`, `read`, `write`, `printf`, `malloc`, `free`, `perror`, `strerror`, `exit` — et toutes les fonctions de la MiniLibX |
| **Libft autorisée** | Oui |

**Description**

Vous devez créer un petit jeu 2D où un dauphin s'échappe de la Terre après avoir
mangé des poissons. Ou n'importe quel héros qui collecte des objets de valeur
avant de quitter les lieux.

Les contraintes sont les suivantes :

- Vous **devez** utiliser la **miniLibX**. Soit la version disponible sur le
  système d'exploitation, soit à partir de ses sources. Si vous choisissez de
  travailler à partir des sources, vous devrez appliquer les mêmes règles pour
  votre `libft` que celles écrites plus haut dans la partie Consignes communes.
- La gestion de votre fenêtre doit rester fluide : passage à une autre fenêtre,
  réduction, etc.
- Les exemples donnés ont un thème de dauphin, mais vous pouvez utiliser ce que
  vous voulez.
- La carte sera construite avec 3 composants : murs, collectibles et espace
  libre.
- L'objectif du joueur est de collecter tous les collectibles présents sur la
  carte, puis de s'échapper avec un minimum de mouvements.
- À chaque mouvement, le nombre courant de mouvements doit être affiché dans le
  shell.
- Le joueur doit pouvoir se déplacer : haut, bas, gauche, droite.
- Vous utiliserez une vue 2D (de dessus ou de profil).
- Le jeu n'a pas besoin d'être en temps réel.
- Le joueur ne peut pas se déplacer dans les murs.
- Le programme affiche l'image dans une fenêtre et respecte les règles
  suivantes :
  - Les touches **W**, **A**, **S** et **D** servent à déplacer le personnage
    principal.
  - Appuyer sur **ESC** doit fermer la fenêtre et quitter le programme
    proprement.
  - Cliquer sur la croix rouge de la fenêtre doit fermer la fenêtre et quitter
    le programme proprement.
  - L'utilisation d'**images** de la **minilibX** est fortement recommandée.
- Votre programme doit prendre en premier argument un fichier de description de
  carte avec l'extension `.ber`.
  - La carte doit être composée de seulement 5 caractères possibles : `0` pour
    un espace vide, `1` pour un mur, `C` pour un collectible, `E` pour la sortie
    et `P` pour la position de départ du joueur.

Ceci est une carte valide simple :

```
1111111111111
10010000000C1
1000011111001
1P0011E000001
1111111111111
```

  - La carte doit être fermée/entourée de murs, sinon le programme doit
    retourner une erreur.
  - La carte doit avoir au moins une sortie, un collectible et une position de
    départ.
  - Vous n'avez pas besoin de vérifier s'il existe un chemin valide dans la
    carte.
  - La carte doit être rectangulaire.
  - Vous devez pouvoir parser n'importe quel type de carte, tant qu'elle
    respecte les règles de la carte.

Une autre carte `.ber` minimale :

```
1111111111111111111111111111111111
1E0000000000000C00000C000000000001
1010010100100000101001000000010101
1010010010101010001001000000010101
1P0000000C00C0000000000000000000C1
1111111111111111111111111111111111
```

  - Si une mauvaise configuration de quelque nature que ce soit est rencontrée
    dans le fichier, le programme doit quitter proprement et retourner
    `"Error\n"` suivi d'un message d'erreur explicite de votre choix.

---

## Chapitre V — Partie bonus

Les bonus ne seront évalués que si votre partie obligatoire est **PARFAITE**.

Par **PARFAITE**, nous entendons naturellement qu'elle doit être complète,
qu'elle ne peut pas échouer, même en cas de mauvaises utilisations. Cela
signifie que si votre partie obligatoire n'obtient pas **TOUS** les points
pendant la notation, vos bonus seront entièrement **IGNORÉS**.

Liste des bonus :

- des patrouilles d'ennemis qui font perdre le joueur en cas de contact.
- il y a une animation de sprite.
- le compteur de mouvements est directement affiché à l'écran au lieu de la
  sortie shell.

Vous pourrez créer de meilleurs jeux plus tard — ne perdez pas trop de temps !

---

## Chapitre VI — Exemples

Figure VI.1 : quelques exemples de `so_long` avec un goût très douteux (presque
digne des bonus) en matière de design graphique !
