# push_swap — partie obligatoire, bonus et benchmark

**Version 10.0** · Transcription française de `docs/en.subject.pdf`

> Transcription fidèle du sujet officiel, restructurée pour la lecture (le PDF
> pagine les exemples de code sur plusieurs pages). Chaque exigence, tableau,
> chiffre du benchmark et exemple de code de l'original est reproduit. En cas de
> doute, le PDF fait foi.

---

## I. Avant-propos

Écrire un programme qui trie des données est l'un des plus vieux exercices de
l'informatique. Le sujet s'ouvre sur un « hello, world » dans une douzaine de
langages — C, ASM, LOLCODE, PHP, BrainFuck, C#, HTML5, YASL, OCaml — pour rappeler
que chaque projet suivant est une variation de la même discipline : exprimer un
processus dans un langage, précisément.

`Push_swap` — *Parce que Swap_push ne sonne pas aussi naturel*.

## II. Introduction

Le projet Push swap est un défi algorithmique simple mais très structuré : il
faut trier des données.

Vous disposez d'un ensemble de valeurs entières, de 2 piles et d'un ensemble
d'instructions pour manipuler les deux piles.

Votre objectif ? Écrire un programme en C nommé `push_swap` qui calcule et
affiche la plus courte séquence d'instructions Push_swap nécessaire pour trier les
entiers donnés.

Facile ? On verra...

## III. Objectifs

Écrire un algorithme de tri est toujours une étape cruciale dans le parcours d'un
développeur. C'est souvent la première rencontre avec la notion de
**complexité**.

Les algorithmes de tri et leur complexité sont des sujets courants en entretien
d'embauche. C'est le bon moment pour explorer ces concepts, vous les rencontrerez
probablement plus tard.

Les objectifs d'apprentissage de ce projet sont la rigueur, la maîtrise du C et
l'application d'algorithmes de base, avec un accent particulier sur leur
complexité.

Trier des valeurs est simple, mais trouver la façon la plus rapide de les trier
est plus délicat. La méthode de tri la plus efficace peut varier selon
l'arrangement des entiers.

## IV. Consignes communes

- Votre projet doit être écrit en C.
- Votre projet doit être écrit en accord avec la Norme. Si vous avez des
  fichiers/fonctions bonus, ils sont inclus dans le contrôle de la norme, et vous
  recevrez un 0 s'il y a une erreur de norme.
- Vos fonctions ne doivent pas quitter de façon inattendue (segmentation fault,
  bus error, double free, etc.) sauf comportement indéfini. Si cela se produit,
  votre projet sera considéré comme non fonctionnel et recevra un 0 pendant
  l'évaluation.
- Toute la mémoire allouée sur le tas doit être correctement libérée quand c'est
  nécessaire. Les fuites de mémoire ne seront pas tolérées.
- Si le sujet l'exige, vous devez rendre un Makefile qui compile vos fichiers
  source vers la sortie demandée avec les drapeaux `-Wall`, `-Wextra` et
  `-Werror`, en utilisant `cc`. De plus, votre Makefile ne doit pas effectuer de
  relink inutile.
- Votre Makefile doit contenir au moins les règles `$(NAME)`, `all`, `clean`,
  `fclean` et `re`.
- Pour rendre des bonus pour votre projet, vous devez inclure une règle `bonus`
  dans votre Makefile, qui ajoutera tous les en-têtes, bibliothèques ou fonctions
  qui ne sont pas autorisés dans la partie principale du projet. Les bonus
  doivent être placés dans des fichiers `_bonus.{c/h}`, sauf si le sujet précise
  autre chose. L'évaluation des parties obligatoire et bonus est menée
  séparément.
- Si votre projet vous permet d'utiliser votre libft, vous devez copier ses
  sources et son Makefile associé dans un dossier `libft`. Le Makefile de votre
  projet doit compiler la bibliothèque en utilisant son Makefile, puis compiler
  le projet.
- Nous vous encourageons à créer des programmes de test pour votre projet, même
  si ce travail n'a pas à être rendu et ne sera pas noté. Cela vous donnera
  l'occasion de tester facilement votre travail et celui de vos pairs. Vous
  trouverez ces tests particulièrement utiles pendant la soutenance. En effet,
  pendant la soutenance, vous êtes libre d'utiliser vos tests et/ou ceux du pair
  que vous évaluez.
- Rendez votre travail dans le dépôt Git assigné. Seul le travail présent dans le
  dépôt Git sera noté. Si Deepthought est chargé de noter votre travail, cela se
  produira après vos évaluations par les pairs. Si une erreur survient dans
  n'importe quelle section de votre travail pendant la notation de Deepthought,
  l'évaluation s'arrêtera.

## V. Consignes sur l'IA

**Contexte.** Pendant votre parcours d'apprentissage, l'IA peut vous aider sur
beaucoup de tâches différentes. Prenez le temps d'explorer les diverses capacités
des outils d'IA et la façon dont ils peuvent soutenir votre travail. Cependant,
approchez-les toujours avec prudence et évaluez critiquement les résultats. Qu'il
s'agisse de code, de documentation, d'idées ou d'explications techniques, vous ne
pouvez jamais être complètement sûr que votre question était bien formulée ou que
le contenu généré est exact. Vos pairs sont une ressource précieuse pour éviter
les erreurs et les angles morts.

**Message principal.**

- Utilisez l'IA pour réduire les tâches répétitives ou fastidieuses.
- Développez des compétences de prompt — en code et hors code — qui serviront
  votre carrière future.
- Apprenez comment fonctionnent les systèmes d'IA pour mieux anticiper et éviter
  les risques, biais et problèmes éthiques courants.
- Continuez à construire des compétences techniques et humaines en travaillant
  avec vos pairs.
- N'utilisez que du contenu généré par IA que vous comprenez entièrement et dont
  vous pouvez assumer la responsabilité.

**Règles de l'apprenant.**

- Vous devriez prendre le temps d'explorer les outils d'IA et de comprendre
  comment ils fonctionnent, afin de les utiliser éthiquement et de réduire les
  biais potentiels.
- Vous devriez réfléchir à votre problème avant de prompter — cela vous aide à
  écrire des prompts plus clairs, plus détaillés et plus pertinents avec un
  vocabulaire précis.
- Vous devriez développer l'habitude de vérifier, relire, questionner et tester
  systématiquement tout ce qui est généré par l'IA.
- Vous devriez toujours rechercher la relecture par un pair — ne vous fiez pas
  uniquement à votre propre validation.

**Résultats de la phase.**

- Développer des compétences de prompt générales et spécifiques au domaine.
- Booster votre productivité par un usage efficace des outils d'IA.
- Continuer à renforcer la pensée computationnelle, la résolution de problèmes,
  l'adaptabilité et la collaboration.

**Commentaires et exemples.**

- Vous rencontrerez régulièrement des situations — examens, évaluations et
  autres — où vous devrez démontrer une compréhension réelle. Soyez prêt,
  continuez à construire vos compétences techniques et interpersonnelles.
- Expliquer votre raisonnement et débattre avec vos pairs révèle souvent des
  lacunes dans votre compréhension. Faites de l'apprentissage entre pairs une
  priorité.
- Les outils d'IA manquent souvent de votre contexte spécifique et tendent à
  fournir des réponses génériques. Vos pairs, qui partagent votre environnement,
  peuvent offrir des éclairages plus pertinents et plus exacts.
- Là où l'IA tend à générer la réponse la plus probable, vos pairs peuvent
  fournir des perspectives alternatives et des nuances précieuses. Appuyez-vous
  sur eux comme point de contrôle qualité.

> **Bonne pratique :** Je demande à l'IA : « Comment tester une fonction de
> tri ? » Elle me donne quelques idées. Je les essaie et je revois les résultats
> avec un pair. Nous affinons l'approche ensemble.
>
> **Mauvaise pratique :** Je demande à l'IA d'écrire une fonction entière et je
> la copie-colle dans mon projet. Pendant l'évaluation par les pairs, je ne peux
> pas expliquer ce qu'elle fait ni pourquoi. Je perds en crédibilité — et
> j'échoue.

> **Bonne pratique :** J'utilise l'IA pour aider à concevoir un parseur. Ensuite,
> je déroule la logique avec un pair. Nous trouvons deux bugs et le réécrivons
> ensemble — mieux, plus propre, et entièrement compris.
>
> **Mauvaise pratique :** Je laisse Copilot générer le code d'une partie clé de
> mon projet. Ça compile, mais je ne peux pas expliquer comment il gère les
> pipes. Pendant l'évaluation, je n'arrive pas à justifier et j'échoue.

## VI. Partie obligatoire

### VI.1 Les règles

- Vous avez **2 piles** nommées `a` et `b`.
- Au départ :
  - La pile `a` contient un nombre aléatoire d'entiers **uniques** négatifs et/ou
    positifs.
  - La pile `b` est vide.
- Le but est de trier les nombres de la pile `a` par ordre croissant. Pour cela,
  vous disposez des opérations suivantes :

| Instruction | Effet |
| :--- | :--- |
| `sa` (swap a) | Échange les 2 premiers éléments du haut de la pile a. Ne fait rien s'il n'y a qu'un élément ou aucun. |
| `sb` (swap b) | Échange les 2 premiers éléments du haut de la pile b. Ne fait rien s'il n'y a qu'un élément ou aucun. |
| `ss` | `sa` et `sb` en même temps. |
| `pa` (push a) | Prend le premier élément du haut de b et le place en haut de a. Ne fait rien si b est vide. |
| `pb` (push b) | Prend le premier élément du haut de a et le place en haut de b. Ne fait rien si a est vide. |
| `ra` (rotate a) | Décale tous les éléments de la pile a vers le haut de 1. Le premier élément devient le dernier. |
| `rb` (rotate b) | Décale tous les éléments de la pile b vers le haut de 1. Le premier élément devient le dernier. |
| `rr` | `ra` et `rb` en même temps. |
| `rra` (reverse rotate a) | Décale tous les éléments de la pile a vers le bas de 1. Le dernier élément devient le premier. |
| `rrb` (reverse rotate b) | Décale tous les éléments de la pile b vers le bas de 1. Le dernier élément devient le premier. |
| `rrr` | `rra` et `rrb` en même temps. |

### VI.2 Exemple

Pour illustrer l'effet de certaines de ces instructions, trions une liste
aléatoire d'entiers. Dans cet exemple, nous considérerons que les deux piles
grandissent vers la droite.

```
Init a et b :
2  1  3  6  5  8   _  _
a           b

Exec sa :
1  2  3  6  5  8   _  _
a           b

Exec pb pb pb :
6  3
5  2
8  1
_  _
a     b

Exec ra rb (équiv. à rr) :
5  2
8  1
6  3
_  _
a     b

Exec rra rrb (équiv. à rrr) :
6  3
5  2
8  1
_  _
a     b

Exec sa :
5  3
6  2
8  1
_  _
a     b

Exec pa pa pa :
1
2
3
5
6
8
_  _
a     b
```

Les entiers de la pile a sont triés en **12 instructions**. Pouvez-vous faire
mieux ?

### VI.3 Le programme « push_swap »

| | |
| :--- | :--- |
| **Nom de programme** | `push_swap` |
| **Fichiers à rendre** | `Makefile`, `*.h`, `*.c` |
| **Makefile** | `NAME`, `all`, `clean`, `fclean`, `re` |
| **Arguments** | pile a : une liste d'entiers |
| **Fonctions externes** | `read`, `write`, `malloc`, `free`, `exit` · `ft_printf` ou tout équivalent **que VOUS avez codé** |
| **Libft autorisée** | Oui |
| **Description** | Trier des piles |

Votre projet doit respecter les règles suivantes :

- Vous devez rendre un Makefile qui compilera vos fichiers source. Il ne doit pas
  effectuer de relink.
- **Les variables globales sont interdites.**
- Vous devez écrire un programme nommé `push_swap` qui prend en argument la pile
  a formatée comme une liste d'entiers. **Le premier argument doit être en haut
  de la pile** (attention à l'ordre).
- Le programme doit afficher la plus courte séquence d'instructions nécessaire
  pour trier la pile a avec le plus petit nombre en haut.
- **Les instructions doivent être séparées par un `\n` et rien d'autre.**
- Le but est de trier la pile avec le plus petit nombre d'opérations possible.
  Pendant l'évaluation, le nombre d'instructions trouvé par votre programme sera
  comparé à une limite : le nombre maximal d'opérations toléré. Si votre
  programme affiche une liste plus longue ou si les nombres ne sont pas
  correctement triés, votre note sera 0.
- Si aucun paramètre n'est spécifié, le programme ne doit rien afficher et doit
  rendre la main.
- En cas d'erreur, il doit afficher `« Error »` suivi d'un `\n` sur la sortie
  d'erreur. Les erreurs incluent par exemple : des arguments qui ne sont pas des
  entiers, des arguments dépassant les limites d'un entier, et/ou la présence de
  doublons.

```
$>./push_swap 2 1 3 6 5 8
sa
pb
pb
pb
sa
pa
pa
pa
$>./push_swap 0 one 2 3
Error
$>
```

Pendant l'évaluation, un binaire sera fourni pour vérifier correctement votre
programme. Il fonctionnera ainsi :

```
$>ARG="4 67 3 87 23"; ./push_swap $ARG | wc -l
6
$>ARG="4 67 3 87 23"; ./push_swap $ARG | ./checker_OS $ARG
OK
$>
```

Si le programme `checker_OS` affiche `« KO »`, cela signifie que votre
`push_swap` a produit une liste d'instructions qui ne trie pas les nombres.

### VI.4 Benchmark

Pour valider ce projet, vous devez effectuer certains tris avec un nombre minimal
d'opérations :

- **Pour une validation maximale du projet (100 %)** et l'éligibilité aux bonus,
  vous devez :
  - Trier **100 nombres aléatoires en moins de 700 opérations**.
  - Trier **500 nombres aléatoires en 5500 opérations maximum**.
- **Pour une validation minimale du projet** (qui implique une note minimale de
  80 %), vous pouvez réussir avec différentes moyennes :
  - 100 nombres en moins de 1100 opérations et 500 nombres en moins de 8500
  - 100 nombres en moins de 700 opérations et 500 nombres en moins de 11500
  - 100 nombres en moins de 1300 opérations et 500 nombres en moins de 5500

Tout cela sera vérifié pendant votre évaluation.

> Si vous souhaitez réaliser la partie bonus, vous devez valider
> scrupuleusement le projet avec chaque étape du benchmark atteignant le meilleur
> score possible.

## VII. Exigences du README

Un fichier `README.md` doit être fourni à la racine de votre dépôt Git. Son but
est de permettre à toute personne non familière avec le projet (pairs, staff,
recruteurs, etc.) de comprendre rapidement de quoi il s'agit, comment l'exécuter,
et où trouver plus d'informations sur le sujet.

Le `README.md` doit inclure au moins :

- La toute première ligne doit être en italique et lire :
  `*This project has been created as part of the 42 curriculum by <login>.*`
- Une section **« Description »** qui présente clairement le projet, incluant son
  but et un bref aperçu.
- Une section **« Instructions »** contenant toute information pertinente sur la
  compilation, l'installation et/ou l'exécution.
- Une section **« Resources »** listant les références classiques liées au sujet
  (documentation, articles, tutoriels, etc.), ainsi qu'une description de la
  façon dont l'IA a été utilisée — en précisant pour quelles tâches et quelles
  parties du projet.

Des sections supplémentaires peuvent être requises selon le projet (ex. exemples
d'utilisation, liste des fonctionnalités, choix techniques, etc.). Toute addition
requise sera explicitement listée ci-dessous.

## VIII. Partie bonus

Du fait de sa simplicité, ce projet offre peu d'opportunités de fonctionnalités
supplémentaires. Cependant, pourquoi ne pas créer votre propre checker ?

Grâce au programme checker, vous pourrez vérifier si la liste d'instructions
générée par le programme `push_swap` trie réellement correctement la pile.

> La partie bonus ne sera évaluée que si la partie obligatoire est parfaite.
> Parfaite signifie que la partie obligatoire a été entièrement réalisée et
> fonctionne sans erreurs. Dans ce projet, cela implique de **valider tous les
> benchmarks sans exception**. Si vous n'avez pas passé TOUTES les exigences
> obligatoires, votre partie bonus ne sera pas évaluée du tout.

### VIII.1 Le programme « checker »

| | |
| :--- | :--- |
| **Nom de programme** | `checker` |
| **Fichiers à rendre** | `*.h`, `*.c` |
| **Makefile** | `bonus` |
| **Arguments** | pile a : une liste d'entiers |
| **Fonctions externes** | `read`, `write`, `malloc`, `free`, `exit` · `ft_printf` ou tout équivalent **que VOUS avez codé** |
| **Libft autorisée** | Oui |
| **Description** | Exécuter les instructions de tri |

- Écrivez un programme nommé `checker` qui prend en argument la pile a formatée
  comme une liste d'entiers. Le premier argument doit être en haut de la pile
  (attention à l'ordre). **Si aucun argument n'est donné, il s'arrête et
  n'affiche rien.**
- Il attend ensuite et lit des instructions sur l'entrée standard, chaque
  instruction suivie de `\n`. Une fois toutes les instructions lues, le programme
  doit les exécuter sur la pile reçue en argument.
- Si après exécution de ces instructions la pile a est réellement triée et la
  pile b est vide, alors le programme doit afficher `« OK »` suivi d'un `\n` sur
  la sortie standard.
- Dans tous les autres cas, il doit afficher `« KO »` suivi d'un `\n` sur la
  sortie standard.
- En cas d'erreur, vous devez afficher `« Error »` suivi d'un `\n` sur la sortie
  d'erreur. Les erreurs incluent par exemple : des arguments qui ne sont pas des
  entiers, des arguments plus grands qu'un entier, des doublons, une instruction
  qui n'existe pas et/ou mal formatée.

```
$>./checker 3 2 1 0
rra
pb
sa
rra
pa
OK
$>./checker 3 2 1 0
sa
rra
pb
KO
$>./checker 3 2 one 0
Error
$>./checker "" 1
Error
$>
```

> Vous n'avez PAS à reproduire exactement le même comportement que le binaire
> fourni. Il est obligatoire de gérer les erreurs, mais c'est à vous de décider
> comment vous voulez analyser les arguments.

## IX. Rendu et évaluation par les pairs

Rendez votre travail dans votre dépôt Git comme d'habitude. Seul le travail à
l'intérieur de votre dépôt sera évalué pendant la soutenance. N'hésitez pas à
vérifier les noms de vos fichiers pour vous assurer qu'ils sont corrects.

Comme ces travaux ne sont pas vérifiés par un programme, organisez vos fichiers
comme vous le souhaitez, tant que vous rendez les fichiers obligatoires et
respectez les exigences.

Pendant l'évaluation, une brève modification du projet peut parfois être
demandée. Cela peut impliquer un changement de comportement mineur, quelques
lignes de code à écrire ou réécrire, ou une fonctionnalité facile à ajouter.

Bien que cette étape ne soit pas applicable à tous les projets, vous devez y être
préparé si elle est mentionnée dans les consignes d'évaluation.

Cette étape vise à vérifier votre compréhension réelle d'une partie spécifique du
projet. La modification peut être effectuée dans n'importe quel environnement de
développement que vous choisissez (par exemple, votre configuration habituelle),
et elle devrait être réalisable en quelques minutes — sauf si un délai spécifique
est défini dans le cadre de l'évaluation.

Vous pouvez, par exemple, être amené à faire une petite mise à jour d'une fonction
ou d'un script, modifier un affichage, ou ajuster une structure de données pour
stocker de nouvelles informations, etc.

Les détails (périmètre, cible, etc.) seront spécifiés dans les consignes
d'évaluation et peuvent varier d'une évaluation à l'autre pour un même projet.
