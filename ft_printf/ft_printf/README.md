# ft_printf

Réimplémentation complète de la fonction `printf` de la libc — projet **ft_printf** de l'école 42.
Partie **obligatoire** + **tous les bonus**, conforme à la Norme, sans fuite mémoire, et validée
par plus de **26 900 tests différentiels** face à la vraie `printf`, ainsi que par le testeur de
référence *Tripouille/printfTester* (mandatory + bonus, sous valgrind).

---

## 1. Sujet

Le sujet officiel (version actuelle) est archivé dans [`docs/ft_printf.en.subject.pdf`](docs/ft_printf.en.subject.pdf).

### Partie obligatoire
- Prototype : `int ft_printf(const char *format, ...);`
- Librairie : `libftprintf.a` (créée avec `ar`, jamais `libtool`).
- Conversions : **`c s p d i u x X %`**.
- Pas de gestion de buffer comme le vrai `printf` (écriture directe via `write`).
- Fonctions externes autorisées : `malloc`, `free`, `write`, `va_start`, `va_arg`, `va_copy`, `va_end`.
- Comparaison avec le vrai `printf`.

### Partie bonus (tous implémentés)
- Flags **`-`** (alignement à gauche), **`0`** (remplissage par zéros), **`.`** (précision) et
  **largeur de champ minimale**, avec **toutes** les conversions.
- Flags **`#`** (forme alternative `0x`/`0X`), **`+`** (signe explicite) et **espace** (signe réservé).
- Largeur et précision dynamiques via **`*`** (y compris valeurs négatives).

---

## 2. Arborescence

```
ft_printf/
├── Makefile                  # all, clean, fclean, re, bonus (sans relink inutile)
├── ft_printf.h               # header public + helpers partagés
├── ft_printf_bonus.h         # structures et prototypes des bonus
├── ft_printf.c               # entrée mandatory (cspdiuxX%)
├── ft_print_cs.c             # %c %s %%
├── ft_print_num.c            # %d %i %u %x %X
├── ft_print_ptr.c            # %p
├── ft_output.c               # write bas niveau + padding + strlen
├── ft_num.c                  # conversion numérique (utoa, numlen)
├── ft_printf_bonus.c         # entrée bonus (parsing + rendu)
├── ft_parse_bonus.c          # parsing flags / largeur / précision / *
├── ft_render_bonus.c         # rendu c / s / % + dispatch
├── ft_render_num_bonus.c     # rendu d / i / u / x / X / p
├── ft_write_number_bonus.c   # alignement / précision / zéros / signe
├── docs/
│   └── ft_printf.en.subject.pdf
├── tests/                    # suite de tests différentielle (voir §4)
└── printfTester/             # testeur de référence Tripouille (outil local, non rendu)
```

> Les fichiers bonus sont bien dans des fichiers séparés suffixés `_bonus.{c,h}`, comme exigé.
> `make` produit la version obligatoire ; `make bonus` produit la version bonus. Les deux sont
> évaluées séparément et ne coexistent jamais dans la même archive.

---

## 3. Compilation

```bash
make            # compile la partie obligatoire -> libftprintf.a
make bonus      # compile la partie bonus       -> libftprintf.a
make clean      # supprime les objets
make fclean     # supprime objets + librairie
make re         # fclean + all
```

Utilisation dans un programme :

```c
#include "ft_printf.h"

int main(void)
{
    ft_printf("Hello %s! %d = %#x, ptr=%p, %+08.3d\n",
              "42", 255, 255, (void *)0xabc, 42);
    return (0);
}
```
```bash
cc main.c -L. -lftprintf -I. -o demo && ./demo
```

---

## 4. Tests

### Suite différentielle (rapide, ~27 000 cas)
Compare `ft_printf` à la vraie `printf` (via `snprintf`) sur un produit cartésien :
**sous-ensembles de flags × largeurs × précisions × valeurs limites** pour chaque conversion,
plus des cas mixtes, les valeurs de retour, les erreurs d'écriture (`/dev/full`) et `NULL`.

```bash
cd tests
make run         # génère les cas, compile et exécute
make valgrind    # même chose sous valgrind (fuites + erreurs mémoire)
make sanitize    # rebuild en -fsanitize=address,undefined puis exécute
```

### Testeur de référence Tripouille (mandatory + bonus, sous valgrind)
```bash
cd printfTester
make m           # tests mandatory
make b           # tests bonus
make a           # les deux
```

### Script global
```bash
./run_tests.sh          # norme + suite différentielle + valgrind + sanitizers
./run_tests.sh --full   # ajoute le testeur Tripouille (mandatory + bonus)
```

---

## 5. Résultats

| Vérification                              | Résultat |
|-------------------------------------------|----------|
| `norminette` (13 fichiers)                | ✅ OK    |
| Compilation `gcc` / `clang` `-Wall -Wextra -Werror` | ✅ OK |
| Tripouille — mandatory (c,s,p,d,i,u,x,X,%,mix) | ✅ 100 % |
| Tripouille — bonus (-,0,.,#,espace,+)     | ✅ 100 % |
| Tests différentiels (~26 921 cas)         | ✅ 0 échec |
| valgrind (fuites / erreurs mémoire)       | ✅ 0     |
| AddressSanitizer + UBSan                  | ✅ 0     |
| Symboles externes (uniquement `write`)    | ✅ conforme |

---

## 6. Notes d'implémentation

- **Aucune allocation dynamique** : tout est rendu sur la pile et écrit directement avec `write`,
  donc aucune fuite possible et aucun appel à `malloc`/`free`.
- `write` peut être partiel : `ft_write_mem` boucle jusqu'à tout écrire ; toute erreur renvoie `-1`,
  propagée comme valeur de retour de `ft_printf`.
- Comportements alignés sur la glibc pour les cas limites testés : `%s` NULL → `(null)`,
  `%p` NULL → `(nil)`, précision `< 6` sur `%s` NULL → vide, `+`/espace sur `%p` non NULL,
  précision qui annule le flag `0`, `#` ignoré pour une valeur hexadécimale nulle, etc.
- `INT_MIN` est géré sans débordement (`-(v + 1) + 1`).
- Le Makefile utilise des « stamps » pour basculer entre mandatory et bonus **sans relink** inutile.
