# Libft — Votre toute première bibliothèque (École 42 — Édition 2026)

> **Référence officielle du sujet** : `Libft — Your very first own library` — **Version 19.3** (Nouveau Tronc Commun / *New Common Core* 42, 2026) — **Norme v4.1**  
> **Résumé** : *Ce projet a pour objectif de vous faire coder en C une bibliothèque regroupant des fonctions usuelles dont vous aurez l'usage dans tous vos prochains projets du cursus.*

---

## Nouveautés majeures du sujet 2026 (Versions 19.2 → 19.3)

Si vous connaissez les anciennes versions du sujet `Libft` (v15, v16, v18), **attention aux changements critiques introduits dans le Nouveau Tronc Commun (v19.2 et v19.3 en 2026)** :

1. **Suppression du chapitre Bonus — Les listes chaînées deviennent la Partie 3 Obligatoire (*Mandatory*)** :
   - Les **9 fonctions de listes chaînées** (`ft_lstnew`, `ft_lstadd_front`, `ft_lstsize`, `ft_lstlast`, `ft_lstadd_back`, `ft_lstdelone`, `ft_lstclear`, `ft_lstiter`, `ft_lstmap`) font désormais **intégralement partie du tronc obligatoire** (Partie 3) et doivent être compilées directement dans `libft.a` par la règle par défaut `make` / `make all` (fichiers nommés `ft_lst*.c` sans suffixe `_bonus`).
   - Le projet compte donc **43 fonctions obligatoires** (23 `libc` + 11 additionnelles + 9 listes chaînées).
2. **Nouveau prototype de `ft_lstsize` en v19.3** :
   - Le type de retour de `ft_lstsize` est passé de `int` (en v19.2 et antérieures) à **`unsigned int`** :
     ```c
     unsigned int ft_lstsize(t_list *lst);
     ```
3. **Valeur de retour stricte `1` ou `0` pour les fonctions de classification de caractères** :
   - Pour `ft_isalpha`, `ft_isdigit`, `ft_isalnum`, `ft_isascii` et `ft_isprint`, le sujet exige désormais explicitement un retour égal à **`1`** lorsque la condition est vraie et **`0`** lorsqu'elle est fausse (alors que la `libc` standard retourne n'importe quelle valeur non nulle).
4. **Spécification explicite de `ft_calloc` pour une taille nulle** :
   - Si `nmemb == 0` ou `size == 0`, `ft_calloc` doit retourner un **pointeur unique pouvant être passé sans erreur à `free()`** (retourner `NULL` dans ce cas est refusé par les moulinettes v19.3). En cas de dépassement arithmétique (`nmemb * size > SIZE_MAX`), `ft_calloc` doit retourner `NULL`.
5. **Ajout du Chapitre III — Directives relatives à l'IA (*AI Instructions*)**.
6. **Ajout d'un fichier `README.md` obligatoire (Chapitre V)** :
   - Un fichier `README.md` rédigé **en anglais** doit être rendu à la racine du dépôt Git avec une première ligne en italique strictement formatée (`*This project has been created as part of the 42 curriculum by <login>.*`) ainsi que les sections obligatoires `Description`, `Instructions` et `Resources` (incluant la déclaration d'usage de l'IA).

---

## Table des matières du sujet officiel (Version 19.3)

- **Chapitre I** — Introduction
- **Chapitre II** — Instructions générales (*Common Instructions*)
- **Chapitre III** — Directives relatives à l'IA (*AI Instructions*)
- **Chapitre IV** — Partie obligatoire (*Mandatory Part*)
  - **IV.1** Considérations techniques (*Technical considerations*)
  - **IV.2** Partie 1 — Fonctions de la `libc` (*Part 1 - Libc functions*)
  - **IV.3** Partie 2 — Fonctions supplémentaires (*Part 2 - Additional functions*)
  - **IV.4** Partie 3 — Listes chaînées (*Part 3 - Linked list*)
- **Chapitre V** — Rendu et peer-évaluation (*Submission and peer-evaluation*) & Exigences du `README.md`

---

## Chapitre I — Introduction

La programmation en C peut s'avérer particulièrement fastidieuse lorsqu'on n'a pas accès aux fonctions standards les plus utiles. Ce projet a pour objectif de vous faire comprendre en profondeur le fonctionnement interne de ces fonctions, de les implémenter vous-même et d'apprendre à les utiliser correctement. Vous allez créer votre propre bibliothèque. Elle vous sera précieuse puisque vous l'utiliserez dans vos prochains projets de C à 42.

Prenez le temps d'enrichir votre `libft` tout au long de l'année. Toutefois, lorsque vous travaillerez sur un nouveau projet, n'oubliez jamais de vérifier que les fonctions utilisées dans votre bibliothèque sont bien autorisées par les consignes dudit projet.

---

## Chapitre II — Instructions générales (*Common Instructions*)

- Votre projet doit être écrit en **C**.
- Votre projet doit être écrit conformément à la **Norme** (Norme v4.1 de 42). Si vous avez des fichiers ou fonctions supplémentaires, ils sont inclus dans la vérification de la Norme et vous aurez **0** en cas d'erreur de norme.
- Vos fonctions ne doivent pas s'arrêter de manière inattendue (*segmentation fault*, *bus error*, *double free*, etc.), mis à part dans le cas d'un comportement indéfini (*undefined behavior*) explicite des fonctions originales. Si cela arrive, votre projet sera considéré comme non fonctionnel et recevra la note de **0** lors de l'évaluation.
- Toute mémoire allouée sur le tas (*heap*) doit être correctement libérée lorsque nécessaire. **Aucune fuite mémoire (*memory leak*) ne sera tolérée.**
- Vous devez rendre un `Makefile` qui compilera vos fichiers sources vers la sortie demandée avec les flags **`-Wall -Wextra -Werror`**, utilisera le compilateur **`cc`**, et ne devra **pas relink** (ne pas recompiler ni recréer l'archive si rien n'a changé).
- Votre `Makefile` doit au minimum contenir les règles **`$(NAME)`**, **`all`**, **`clean`**, **`fclean`** et **`re`**.
- Si un projet futur vous autorise à utiliser votre `libft`, vous devrez copier ses sources et son `Makefile` associé dans un dossier `libft/`. Le `Makefile` du projet devra compiler la bibliothèque en appelant son `Makefile`, puis compiler le projet.
- Nous vous encourageons vivement à créer des programmes de test pour votre projet, même si ce travail **ne doit pas être rendu dans le dépôt final et ne sera pas noté**. Cela vous permettra de tester facilement votre travail ainsi que celui de vos pairs lors des soutenances (*peer-evaluations*).
- Rendez votre travail sur le dépôt Git qui vous est assigné. Seul le travail présent sur le dépôt Git sera évalué. Si *Deepthought* (la Moulinette) évalue votre travail, cela aura lieu après vos peer-évaluations. Si une erreur survient dans n'importe quelle section lors de la notation par *Deepthought*, l'évaluation s'arrêtera immédiatement.

---

## Chapitre III — Directives relatives à l'IA (*AI Instructions*)

### Contexte
Durant votre parcours d'apprentissage, l'IA peut vous assister dans de nombreuses tâches différentes. Prenez le temps d'explorer les capacités des outils d'IA et la manière dont ils peuvent soutenir votre travail, tout en gardant un esprit critique : l'apprentissage à 42 repose avant tout sur votre propre capacité à raisonner, concevoir, déboguer et expliquer chaque ligne de code.

### Règles principales
- **Interdiction des raccourcis aveugles** : N'utilisez pas l'IA pour générer du code que vous ne comprenez pas intégralement. Utiliser du code généré sans maîtrise mène systématiquement à l'échec en soutenance ou aux examens (où aucun accès internet ni IA n'est disponible).
- **Usage recommandé** : Utilisez l'IA comme un tuteur socratique pour clarifier des concepts bas niveau (pointeurs, pile vs tas, alignement mémoire, complexité), explorer des cas limites (*edge cases*), relire votre code ou structurer votre documentation.
- **Responsabilité en peer-évaluation** : Vous devez être capable d'expliquer, justifier et réécrire en direct chaque fonction, chaque choix d'allocation et chaque condition de votre projet devant vos évaluateurs.
- **Transparence obligatoire** : Dans la section `Resources` de votre `README.md`, vous devez documenter clairement si et comment l'IA a été utilisée (pour quelles tâches et sur quelles parties du projet).

---

## Chapitre IV — Partie obligatoire (*Mandatory Part*)

| Champ | Spécification officielle |
| :--- | :--- |
| **Nom du programme / archive** | `libft.a` |
| **Fichiers de rendu** | `Makefile`, `libft.h`, `ft_*.c`, `README.md` |
| **Règles du Makefile** | `NAME`, `all`, `clean`, `fclean`, `re` |
| **Fonctions externes autorisées** | Détaillées ci-dessous (`malloc`, `free`, `write` uniquement là où spécifié) |
| **Libft autorisée** | `n/a` |
| **Description** | Écrire votre propre bibliothèque : une collection de fonctions qui constituera un outil utile tout au long de votre cursus. |

### IV.1 Considérations techniques (*Technical considerations*)

- Déclarer des **variables globales est strictement interdit** (une variable `static` déclarée en dehors d'une fonction est également considérée comme globale et interdite).
- Si vous avez besoin de sous-fonctions (*helper functions*) pour découper une fonction complexe, définissez-les obligatoirement avec le mot-clé **`static`**. Ainsi, leur portée restera limitée au fichier concerné.
- Placez **tous vos fichiers à la racine** de votre dépôt.
- Rendre des **fichiers inutilisés est interdit**.
- Tous les fichiers `.c` doivent compiler avec les flags **`-Wall -Wextra -Werror`**.
- Vous devez utiliser la commande **`ar`** pour créer votre bibliothèque statique (`ar rcs libft.a ...`). L'utilisation de la commande **`libtool` est interdite**.
- Votre fichier **`libft.a` doit être créé à la racine** de votre dépôt.

---

### IV.2 Partie 1 — Fonctions de la `libc` (*23 fonctions*)

Pour commencer, vous devez réimplémenter un ensemble de fonctions de la `libc`. Vos fonctions auront les **mêmes prototypes** et implémenteront les **mêmes comportements** que les originales telles que décrites dans leur page de manuel (`man 3`), à l'exception des précisions explicites ci-dessous. La seule différence de signature sera leur nom, préfixé par **`ft_`** (par exemple, `strlen` devient `ft_strlen`).

> **Restriction C99 (`restrict`)** : Certains prototypes originaux dans les pages de `man` utilisent le qualificateur `restrict`. Ce mot-clé fait partie du standard C99 : il est **interdit** de l'inclure dans vos propres prototypes et interdit de compiler votre code avec le flag `-std=c99`.

> **Exigence stricte v19.2 / v19.3 sur les fonctions de classification (`ft_is*`)** :  
> Pour les cinq fonctions de classification de caractères (`ft_isalpha`, `ft_isdigit`, `ft_isalnum`, `ft_isascii`, `ft_isprint`), la valeur de retour doit obligatoirement être :
> - **`1`** si le caractère correspond à la classe testée ;
> - **`0`** si le caractère ne correspond pas.

#### 1. Fonctions sans aucune fonction externe autorisée (`External functs.: None`)

| # | Fonction | Prototype exact dans `libft.h` | Description & Comportement attendu |
| :-: | :--- | :--- | :--- |
| 1 | `ft_isalpha` | `int ft_isalpha(int c);` | Teste si `c` est une lettre alphabétique ASCII (`'A'..'Z'` ou `'a'..'z'`). Retourne exactement `1` si vrai, `0` sinon. |
| 2 | `ft_isdigit` | `int ft_isdigit(int c);` | Teste si `c` est un chiffre décimal ASCII (`'0'..'9'`). Retourne exactement `1` si vrai, `0` sinon. |
| 3 | `ft_isalnum` | `int ft_isalnum(int c);` | Teste si `c` est alphanumérique (`ft_isalpha(c) \|\| ft_isdigit(c)`). Retourne exactement `1` si vrai, `0` sinon. |
| 4 | `ft_isascii` | `int ft_isascii(int c);` | Teste si `c` appartient au jeu US-ASCII 7 bits (`0 <= c && c <= 127`). Retourne exactement `1` si vrai, `0` sinon. |
| 5 | `ft_isprint` | `int ft_isprint(int c);` | Teste si `c` est un caractère imprimable ASCII, espace inclus (`32 <= c && c <= 126`). Retourne exactement `1` si vrai, `0` sinon. |
| 6 | `ft_strlen` | `size_t ft_strlen(const char *s);` | Calcule la longueur de la chaîne `s` (sans compter l'octet nul final `'\0'`). |
| 7 | `ft_memset` | `void *ft_memset(void *s, int c, size_t n);` | Remplit les `n` premiers octets de la zone mémoire pointée par `s` avec l'octet `(unsigned char)c`. Retourne `s`. |
| 8 | `ft_bzero` | `void ft_bzero(void *s, size_t n);` | Met à zéro (`'\0'`) les `n` premiers octets de la zone pointée par `s`. Si `n == 0`, ne fait rien. |
| 9 | `ft_memcpy` | `void *ft_memcpy(void *dest, const void *src, size_t n);` | Copie `n` octets depuis `src` vers `dest` (zones non chevauchantes). Si `n == 0` ou `dest == src`, retourne `dest` sans déréférencement. Retourne `dest`. |
| 10 | `ft_memmove` | `void *ft_memmove(void *dest, const void *src, size_t n);` | Copie `n` octets de `src` vers `dest` en gérant de façon sûre le **chevauchement** des zones mémoire (copie arrière si `dest > src`, avant sinon). Retourne `dest`. |
| 11 | `ft_strlcpy` | `size_t ft_strlcpy(char *dst, const char *src, size_t size);` | Copie au plus `size - 1` octets de `src` vers `dst` et termine par `'\0'` si `size > 0`. Retourne toujours `ft_strlen(src)`. |
| 12 | `ft_strlcat` | `size_t ft_strlcat(char *dst, const char *src, size_t size);` | Ajoute `src` à la fin de `dst` dans une taille tampon totale `size`, en terminant par `'\0'` (sauf si `size == 0` ou aucun `'\0'` trouvé dans les `size` premiers octets de `dst`). Ne lit jamais plus de `size` octets dans `dst`. Retourne `initial_dst_len + ft_strlen(src)` (avec `initial_dst_len` plafonné à `size`). |
| 13 | `ft_toupper` | `int ft_toupper(int c);` | Convertit une lettre minuscule ASCII (`'a'..'z'`) en majuscule (`'A'..'Z'`), sinon retourne `c` inchangé. |
| 14 | `ft_tolower` | `int ft_tolower(int c);` | Convertit une lettre majuscule ASCII (`'A'..'Z'`) en minuscule (`'a'..'z'`), sinon retourne `c` inchangé. |
| 15 | `ft_strchr` | `char *ft_strchr(const char *s, int c);` | Recherche la **première** occurrence de `(char)c` dans `s`. Le `'\0'` final fait partie de la chaîne (si `(char)c == '\0'`, retourne un pointeur sur le `'\0'`). Retourne `NULL` si introuvable. |
| 16 | `ft_strrchr` | `char *ft_strrchr(const char *s, int c);` | Recherche la **dernière** occurrence de `(char)c` dans `s` (`'\0'` inclus). Retourne `NULL` si introuvable. |
| 17 | `ft_strncmp` | `int ft_strncmp(const char *s1, const char *s2, size_t n);` | Compare au plus `n` octets de `s1` et `s2` interprétés comme **`unsigned char`**. S'arrête au premier octet différent, à `'\0'`, ou après `n` octets (`n == 0` retourne `0`). |
| 18 | `ft_memchr` | `void *ft_memchr(const void *s, int c, size_t n);` | Examine les `n` premiers octets de `s` à la recherche de la première occurrence de `(unsigned char)c`. Retourne un pointeur sur l'octet correspondant ou `NULL`. |
| 19 | `ft_memcmp` | `int ft_memcmp(const void *s1, const void *s2, size_t n);` | Compare les `n` premiers octets des zones `s1` et `s2` en tant que **`unsigned char`**. Retourne `0` si `n == 0` ou zones identiques, sinon `p1[i] - p2[i]`. |
| 20 | `ft_strnstr` | `char *ft_strnstr(const char *big, const char *little, size_t len);` | Cherche la première occurrence de la sous-chaîne `little` dans `big`, en n'examinant pas plus de `len` caractères. Si `little` est vide (`""`), retourne `(char *)big`. Si introuvable dans les `len` premiers octets, retourne `NULL`. |
| 21 | `ft_atoi` | `int ft_atoi(const char *nptr);` | Ignore les caractères blancs (`' '`, `'\t'`, `'\n'`, `'\v'`, `'\f'`, `'\r'`), lit au plus un signe optionnel `'+'` ou `'-'`, puis convertit la suite de chiffres décimaux en `int`. |

#### 2. Fonctions utilisant `malloc()` (`External functs.: malloc`)

| # | Fonction | Prototype exact dans `libft.h` | Description & Comportement attendu |
| :-: | :--- | :--- | :--- |
| 22 | `ft_calloc` | `void *ft_calloc(size_t nmemb, size_t size);` | Alloue la mémoire pour un tableau de `nmemb` éléments de `size` octets chacun et initialise tous les octets à zéro. **Si `nmemb == 0` ou `size == 0`, retourne un pointeur unique libérable par `free()`**. Si `nmemb * size` dépasse `SIZE_MAX` (overflow) ou si l'allocation échoue, retourne `NULL`. |
| 23 | `ft_strdup` | `char *ft_strdup(const char *s);` | Alloue (avec `malloc(3)`) la mémoire exacte (`ft_strlen(s) + 1`) et retourne une copie terminée par `'\0'` de la chaîne `s`, ou `NULL` si l'allocation échoue. |

---

### IV.3 Partie 2 — Fonctions supplémentaires (*11 fonctions*)

Dans cette deuxième partie, vous devez développer un ensemble de fonctions qui ne sont soit pas présentes dans la `libc`, soit présentes sous une forme différente.

> *Note : Certaines fonctions de la Partie 1 peuvent bien entendu être réutilisées pour implémenter celles de la Partie 2.*

#### 24. `ft_substr`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `char *ft_substr(char const *s, unsigned int start, size_t len);` |
| **Paramètres** | `s` : La chaîne de laquelle extraire la nouvelle chaîne.<br>`start` : L'index de début de la nouvelle chaîne dans la chaîne `s`.<br>`len` : La taille maximale de la nouvelle chaîne. |
| **Valeur de retour** | La nouvelle chaîne extraite. `NULL` si l'allocation échoue (ou si `s == NULL`). |
| **Fonctions externes** | `malloc` |
| **Description** | Alloue (avec `malloc(3)`) et retourne une chaîne de caractères issue de la chaîne `s`. Cette nouvelle chaîne commence à l'index `start` et a pour taille maximale `len`. *(Si `start >= ft_strlen(s)`, retourne une chaîne vide `""` allouée de 1 octet ; n'alloue jamais plus que le nécessaire + `'\0'`.)* |

#### 25. `ft_strjoin`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `char *ft_strjoin(char const *s1, char const *s2);` |
| **Paramètres** | `s1` : La chaîne préfixe.<br>`s2` : La chaîne suffixe. |
| **Valeur de retour** | La nouvelle chaîne. `NULL` si l'allocation échoue (ou si `s1 == NULL` ou `s2 == NULL`). |
| **Fonctions externes** | `malloc` |
| **Description** | Alloue (avec `malloc(3)`) et retourne une nouvelle chaîne, résultat de la concaténation de `s1` et `s2`. |

#### 26. `ft_strtrim`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `char *ft_strtrim(char const *s1, char const *set);` |
| **Paramètres** | `s1` : La chaîne de caractères à trimmer.<br>`set` : Le jeu de caractères de référence à retirer. |
| **Valeur de retour** | La chaîne trimmée. `NULL` si l'allocation échoue (ou si `s1 == NULL` ou `set == NULL`). |
| **Fonctions externes** | `malloc` |
| **Description** | Alloue (avec `malloc(3)`) et retourne une copie de la chaîne `s1`, sans les caractères spécifiés dans `set` au début et à la fin de la chaîne de caractères. |

#### 27. `ft_split`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `char **ft_split(char const *s, char c);` |
| **Paramètres** | `s` : La chaîne de caractères à découper.<br>`c` : Le caractère délimiteur. |
| **Valeur de retour** | Le tableau de nouvelles chaînes résultant du découpage. `NULL` si l'allocation échoue (ou si `s == NULL`). |
| **Fonctions externes** | `malloc`, `free` |
| **Description** | Alloue (avec `malloc(3)`) et retourne un tableau de chaînes de caractères obtenu en séparant `s` à l'aide du caractère `c`, utilisé comme délimiteur. Le tableau doit être terminé par un pointeur `NULL`. *(En cas d'échec d'une allocation intermédiaire, toutes les chaînes déjà allouées ainsi que le tableau doivent être libérés avec `free(3)` avant de retourner `NULL`.)* |

#### 28. `ft_itoa`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `char *ft_itoa(int n);` |
| **Paramètres** | `n` : L'entier à convertir. |
| **Valeur de retour** | La chaîne de caractères représentant l'entier. `NULL` si l'allocation échoue. |
| **Fonctions externes** | `malloc` |
| **Description** | Alloue (avec `malloc(3)`) et retourne une chaîne de caractères représentant l'entier `n` reçu en argument. Les nombres négatifs doivent être gérés (y compris `INT_MIN = -2147483648`). |

#### 29. `ft_strmapi`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `char *ft_strmapi(char const *s, char (*f)(unsigned int, char));` |
| **Paramètres** | `s` : La chaîne sur laquelle itérer.<br>`f` : La fonction à appliquer à chaque caractère. |
| **Valeur de retour** | La chaîne créée par les applications successives de `f`. Retourne `NULL` si l'allocation échoue (ou si `s == NULL` ou `f == NULL`). |
| **Fonctions externes** | `malloc` |
| **Description** | Applique la fonction `f` à chaque caractère de la chaîne `s`, en passant son index comme premier argument et le caractère lui-même comme second argument, afin de créer une nouvelle chaîne (avec `malloc(3)`) résultant des applications successives de `f`. |

#### 30. `ft_striteri`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_striteri(char *s, void (*f)(unsigned int, char*));` |
| **Paramètres** | `s` : La chaîne sur laquelle itérer.<br>`f` : La fonction à appliquer à chaque caractère. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | Aucune (`None`). |
| **Description** | Applique la fonction `f` à chaque caractère de la chaîne de caractères transmise en argument, en passant son index comme premier argument. Chaque caractère est transmis par adresse à `f` afin d'être modifié si nécessaire. |

#### 31. `ft_putchar_fd`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_putchar_fd(char c, int fd);` |
| **Paramètres** | `c` : Le caractère à écrire.<br>`fd` : Le descripteur de fichier sur lequel écrire. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | `write` |
| **Description** | Écrit le caractère `c` sur le descripteur de fichier donné. |

#### 32. `ft_putstr_fd`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_putstr_fd(char *s, int fd);` |
| **Paramètres** | `s` : La chaîne de caractères à écrire.<br>`fd` : Le descripteur de fichier sur lequel écrire. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | `write` |
| **Description** | Écrit la chaîne de caractères `s` sur le descripteur de fichier donné. |

#### 33. `ft_putendl_fd`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_putendl_fd(char *s, int fd);` |
| **Paramètres** | `s` : La chaîne de caractères à écrire.<br>`fd` : Le descripteur de fichier sur lequel écrire. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | `write` |
| **Description** | Écrit la chaîne de caractères `s` sur le descripteur de fichier donné, suivie d'un retour à la ligne (`'\n'`). |

#### 34. `ft_putnbr_fd`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_putnbr_fd(int n, int fd);` |
| **Paramètres** | `n` : L'entier à écrire.<br>`fd` : Le descripteur de fichier sur lequel écrire. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | `write` |
| **Description** | Écrit l'entier `n` sur le descripteur de fichier donné (doit gérer `0`, les négatifs, `INT_MIN` et `INT_MAX`). |

---

### IV.4 Partie 3 — Listes chaînées (*9 fonctions — Obligatoire en v19.3*)

Manipuler la mémoire et les chaînes de caractères est très utile, mais vous découvrirez vite que manipuler des listes chaînées l'est encore davantage. Dans la version 2026 du sujet (**v19.2 / v19.3**), cette partie est **obligatoire** et compilée directement dans `libft.a` avec `make`.

Vous devez utiliser la structure suivante pour représenter un maillon (*node*) de votre liste. Ajoutez sa déclaration à votre fichier **`libft.h`** :

```c
typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;
```

Les membres de la structure `t_list` sont :
- **`content`** : La donnée contenue dans le maillon. Le `void *` permet de stocker une donnée de n'importe quel type.
- **`next`** : L'adresse du maillon suivant de la liste, ou `NULL` si le maillon est le dernier.

#### 35. `ft_lstnew`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `t_list *ft_lstnew(void *content);` |
| **Paramètres** | `content` : Le contenu avec lequel initialiser le nouveau maillon. |
| **Valeur de retour** | Le nouveau maillon (`NULL` si l'allocation échoue). |
| **Fonctions externes** | `malloc` |
| **Description** | Alloue (avec `malloc(3)`) et retourne un nouveau maillon. La variable membre `content` est initialisée à l'aide de la valeur du paramètre `content` (même si `content == NULL`). La variable `next` est initialisée à `NULL`. |

#### 36. `ft_lstadd_front`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_lstadd_front(t_list **lst, t_list *new);` |
| **Paramètres** | `lst` : L'adresse du pointeur vers le premier maillon d'une liste.<br>`new` : L'adresse du pointeur vers le maillon à ajouter à la liste. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | Aucune (`None`). |
| **Description** | Ajoute le maillon `new` au début de la liste. |

#### 37. `ft_lstsize` *(Modifié en v19.3 : retourne `unsigned int`)*
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `unsigned int ft_lstsize(t_list *lst);` |
| **Paramètres** | `lst` : Le début de la liste. |
| **Valeur de retour** | La taille de la liste (`unsigned int`). |
| **Fonctions externes** | Aucune (`None`). |
| **Description** | Compte le nombre de maillons de la liste (`0` si `lst == NULL`). |

#### 38. `ft_lstlast`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `t_list *ft_lstlast(t_list *lst);` |
| **Paramètres** | `lst` : Le début de la liste. |
| **Valeur de retour** | Dernier maillon de la liste (`NULL` si la liste est vide). |
| **Fonctions externes** | Aucune (`None`). |
| **Description** | Retourne le dernier maillon de la liste. |

#### 39. `ft_lstadd_back`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_lstadd_back(t_list **lst, t_list *new);` |
| **Paramètres** | `lst` : L'adresse du pointeur vers le premier maillon d'une liste.<br>`new` : L'adresse du pointeur vers le maillon à ajouter à la liste. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | Aucune (`None`). |
| **Description** | Ajoute le maillon `new` à la fin de la liste (si `*lst == NULL`, `new` devient le premier maillon). |

#### 40. `ft_lstdelone`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_lstdelone(t_list *lst, void (*del)(void *));` |
| **Paramètres** | `lst` : Le maillon à libérer.<br>`del` : L'adresse de la fonction permettant de supprimer le contenu. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | `free` |
| **Description** | Prend en paramètre un maillon et libère la mémoire du contenu du maillon en utilisant la fonction `del` donnée en paramètre, puis libère le maillon lui-même avec `free(3)`. **La mémoire de `next` ne doit pas être libérée.** |

#### 41. `ft_lstclear`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_lstclear(t_list **lst, void (*del)(void *));` |
| **Paramètres** | `lst` : L'adresse du pointeur vers un maillon.<br>`del` : L'adresse de la fonction permettant de supprimer le contenu d'un maillon. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | `free` |
| **Description** | Supprime et libère la mémoire du maillon passé en paramètre et de tous les maillons qui suivent, à l'aide de `del` et de `free(3)`. Enfin, **le pointeur initial doit être mis à `NULL`**. |

#### 42. `ft_lstiter`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `void ft_lstiter(t_list *lst, void (*f)(void *));` |
| **Paramètres** | `lst` : L'adresse du pointeur vers un maillon.<br>`f` : L'adresse de la fonction à appliquer au contenu de chaque maillon. |
| **Valeur de retour** | Aucune (`void`). |
| **Fonctions externes** | Aucune (`None`). |
| **Description** | Itère sur la liste `lst` et applique la fonction `f` au contenu de chaque maillon. |

#### 43. `ft_lstmap`
| Champ | Spécification |
| :--- | :--- |
| **Prototype** | `t_list *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));` |
| **Paramètres** | `lst` : L'adresse du pointeur vers un maillon.<br>`f` : L'adresse de la fonction à appliquer.<br>`del` : L'adresse de la fonction permettant de supprimer le contenu d'un maillon si nécessaire. |
| **Valeur de retour** | La nouvelle liste. `NULL` si l'allocation échoue. |
| **Fonctions externes** | `malloc`, `free` |
| **Description** | Itère sur la liste `lst` et applique la fonction `f` au contenu de chaque maillon. Crée une nouvelle liste résultant des applications successives de `f`. La fonction `del` est là pour détruire le contenu d'un maillon si nécessaire (en cas d'échec d'allocation d'un nouveau maillon, le contenu courant créé par `f` ainsi que toute la nouvelle liste partielle doivent être libérés sans fuite mémoire, tout en laissant la liste originale intacte). |

---

## Chapitre V — Rendu, Peer-Évaluation & Exigences du `README.md`

### 1. Règles de dépôt
- Rendez votre travail dans votre dépôt Git habituel (Vogsphere). Seul le travail présent dans votre dépôt sera évalué lors de la soutenance.
- Vérifiez scrupuleusement le nom exact de tous vos fichiers (`Makefile`, `libft.h`, `README.md` et les 43 fichiers `ft_*.c`).
- Placez **tous vos fichiers à la racine** de votre dépôt.

### 2. Spécification obligatoire du fichier `README.md`
Un fichier **`README.md`** doit être fourni à la racine de votre dépôt Git. Son objectif est de permettre à toute personne n'ayant aucune connaissance préalable du projet (pairs, équipe pédagogique, recruteurs) de comprendre rapidement de quoi il s'agit, comment le compiler et quelles sont les ressources liées.

Le `README.md` doit impérativement respecter les règles suivantes :
- Être rédigé **en anglais**.
- Sa **toute première ligne** doit être en **italique** et lire exactement :
  ```markdown
  *This project has been created as part of the 42 curriculum by <login>.*
  ```
- Contenir une section **`Description`** présentant clairement le projet, son objectif, une vue d'ensemble ainsi qu'une description détaillée du contenu de la bibliothèque.
- Contenir une section **`Instructions`** regroupant toutes les informations pertinentes sur les prérequis, la compilation (`make`, `make clean`, `make fclean`, `make re`) et l'intégration de `libft.a` dans un programme C.
- Contenir une section **`Resources`** listant les références classiques liées au sujet (pages `man`, documentation standard C, Norme 42), ainsi qu'une description explicite de **la manière dont l'IA a été utilisée** (en précisant pour quelles tâches et sur quelles parties du projet).
