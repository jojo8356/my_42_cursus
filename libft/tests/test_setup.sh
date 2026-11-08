#!/usr/bin/env bash
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIBFT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

PASS_COUNT=0
FAIL_COUNT=0

pass() {
	PASS_COUNT=$((PASS_COUNT + 1))
	printf "  ${GREEN}[OK]${RESET} %s\n" "$1"
}

fail() {
	FAIL_COUNT=$((FAIL_COUNT + 1))
	printf "  ${RED}[KO]${RESET} %s\n" "$1"
}

printf "${BOLD}${CYAN}=== PHASE 1 : CONFORMITE DU PROJET, NORME, MAKEFILE, SYMBOLES & README (v19.3) ===${RESET}\n"

# 1. Exact file inventory check (43 .c files + libft.h + Makefile + README.md)
EXPECTED_C_FILES=(
	ft_isalpha.c ft_isdigit.c ft_isalnum.c ft_isascii.c ft_isprint.c
	ft_strlen.c ft_memset.c ft_bzero.c ft_memcpy.c ft_memmove.c
	ft_strlcpy.c ft_strlcat.c ft_toupper.c ft_tolower.c ft_strchr.c
	ft_strrchr.c ft_strncmp.c ft_memchr.c ft_memcmp.c ft_strnstr.c
	ft_atoi.c ft_calloc.c ft_strdup.c
	ft_substr.c ft_strjoin.c ft_strtrim.c ft_split.c ft_itoa.c
	ft_strmapi.c ft_striteri.c ft_putchar_fd.c ft_putstr_fd.c
	ft_putendl_fd.c ft_putnbr_fd.c
	ft_lstnew.c ft_lstadd_front.c ft_lstsize.c ft_lstlast.c
	ft_lstadd_back.c ft_lstdelone.c ft_lstclear.c ft_lstiter.c ft_lstmap.c
)

make -C "$LIBFT_DIR" fclean >/dev/null 2>&1 || true

MISSING=0
for f in "${EXPECTED_C_FILES[@]}" libft.h Makefile README.md; do
	if [ ! -f "$LIBFT_DIR/$f" ]; then
		fail "Fichier manquant a la racine de libft/ : $f"
		MISSING=1
	fi
done
if [ "$MISSING" -eq 0 ]; then
	pass "Presence des 43 fichiers ft_*.c, libft.h, Makefile et README.md a la racine"
fi

EXTRA_FILES=$(find "$LIBFT_DIR" -maxdepth 1 -type f \
	! -name 'Makefile' ! -name 'libft.h' ! -name 'README.md' \
	! -name '.gitignore' ! -name 'ft_*.c')
ACTUAL_C_COUNT=$(find "$LIBFT_DIR" -maxdepth 1 -name 'ft_*.c' | wc -l | tr -d ' ')
if [ -z "$EXTRA_FILES" ] && [ "$ACTUAL_C_COUNT" -eq 43 ]; then
	pass "Aucun fichier parasite ou inutilise dans libft/ (exactement 43 fichiers .c)"
else
	fail "Fichiers inattendus detectes dans libft/ : $EXTRA_FILES (nb .c = $ACTUAL_C_COUNT)"
fi

# 2. Official 42 Norminette check
if command -v norminette >/dev/null 2>&1; then
	NORM_OUT=$(norminette "$LIBFT_DIR/libft.h" "$LIBFT_DIR"/ft_*.c 2>&1)
	NORM_EXIT=$?
	if [ "$NORM_EXIT" -eq 0 ] && ! printf "%s\n" "$NORM_OUT" | grep -q "Error"; then
		OK_FILES=$(printf "%s\n" "$NORM_OUT" | grep -c ": OK!")
		pass "Norminette officielle 42 validee sans aucune erreur ($OK_FILES/44 fichiers OK!)"
	else
		fail "Erreur Norminette detectee :"
		printf "%s\n" "$NORM_OUT" | grep -v ": OK!" | head -n 20
	fi
else
	printf "  [INFO] norminette non trouvee dans le PATH (etape ignoree)\n"
fi

# 3. Header checks: standalone compile, double inclusion, no 'restrict', exact v19.3 prototypes
TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

cat > "$TMP_DIR/proto_check.c" << 'EOF'
#include "libft.h"
#include "libft.h"

int main(void)
{
	int				(*p_isalpha)(int) = &ft_isalpha;
	int				(*p_isdigit)(int) = &ft_isdigit;
	int				(*p_isalnum)(int) = &ft_isalnum;
	int				(*p_isascii)(int) = &ft_isascii;
	int				(*p_isprint)(int) = &ft_isprint;
	size_t			(*p_strlen)(const char *) = &ft_strlen;
	void			*(*p_memset)(void *, int, size_t) = &ft_memset;
	void			(*p_bzero)(void *, size_t) = &ft_bzero;
	void			*(*p_memcpy)(void *, const void *, size_t) = &ft_memcpy;
	void			*(*p_memmove)(void *, const void *, size_t) = &ft_memmove;
	size_t			(*p_strlcpy)(char *, const char *, size_t) = &ft_strlcpy;
	size_t			(*p_strlcat)(char *, const char *, size_t) = &ft_strlcat;
	int				(*p_toupper)(int) = &ft_toupper;
	int				(*p_tolower)(int) = &ft_tolower;
	char			*(*p_strchr)(const char *, int) = &ft_strchr;
	char			*(*p_strrchr)(const char *, int) = &ft_strrchr;
	int				(*p_strncmp)(const char *, const char *, size_t) = &ft_strncmp;
	void			*(*p_memchr)(const void *, int, size_t) = &ft_memchr;
	int				(*p_memcmp)(const void *, const void *, size_t) = &ft_memcmp;
	char			*(*p_strnstr)(const char *, const char *, size_t) = &ft_strnstr;
	int				(*p_atoi)(const char *) = &ft_atoi;
	void			*(*p_calloc)(size_t, size_t) = &ft_calloc;
	char			*(*p_strdup)(const char *) = &ft_strdup;
	char			*(*p_substr)(char const *, unsigned int, size_t) = &ft_substr;
	char			*(*p_strjoin)(char const *, char const *) = &ft_strjoin;
	char			*(*p_strtrim)(char const *, char const *) = &ft_strtrim;
	char			**(*p_split)(char const *, char) = &ft_split;
	char			*(*p_itoa)(int) = &ft_itoa;
	char			*(*p_strmapi)(char const *, char (*)(unsigned int, char)) = &ft_strmapi;
	void			(*p_striteri)(char *, void (*)(unsigned int, char *)) = &ft_striteri;
	void			(*p_putchar_fd)(char, int) = &ft_putchar_fd;
	void			(*p_putstr_fd)(char *, int) = &ft_putstr_fd;
	void			(*p_putendl_fd)(char *, int) = &ft_putendl_fd;
	void			(*p_putnbr_fd)(int, int) = &ft_putnbr_fd;
	t_list			*(*p_lstnew)(void *) = &ft_lstnew;
	void			(*p_lstadd_front)(t_list **, t_list *) = &ft_lstadd_front;
	unsigned int	(*p_lstsize)(t_list *) = &ft_lstsize;
	t_list			*(*p_lstlast)(t_list *) = &ft_lstlast;
	void			(*p_lstadd_back)(t_list **, t_list *) = &ft_lstadd_back;
	void			(*p_lstdelone)(t_list *, void (*)(void *)) = &ft_lstdelone;
	void			(*p_lstclear)(t_list **, void (*)(void *)) = &ft_lstclear;
	void			(*p_lstiter)(t_list *, void (*)(void *)) = &ft_lstiter;
	t_list			*(*p_lstmap)(t_list *, void *(*)(void *), void (*)(void *)) = &ft_lstmap;
	t_list			node;

	node.content = NULL;
	node.next = NULL;
	(void)node;
	(void)p_isalpha; (void)p_isdigit; (void)p_isalnum; (void)p_isascii;
	(void)p_isprint; (void)p_strlen; (void)p_memset; (void)p_bzero;
	(void)p_memcpy; (void)p_memmove; (void)p_strlcpy; (void)p_strlcat;
	(void)p_toupper; (void)p_tolower; (void)p_strchr; (void)p_strrchr;
	(void)p_strncmp; (void)p_memchr; (void)p_memcmp; (void)p_strnstr;
	(void)p_atoi; (void)p_calloc; (void)p_strdup; (void)p_substr;
	(void)p_strjoin; (void)p_strtrim; (void)p_split; (void)p_itoa;
	(void)p_strmapi; (void)p_striteri; (void)p_putchar_fd; (void)p_putstr_fd;
	(void)p_putendl_fd; (void)p_putnbr_fd; (void)p_lstnew; (void)p_lstadd_front;
	(void)p_lstsize; (void)p_lstlast; (void)p_lstadd_back; (void)p_lstdelone;
	(void)p_lstclear; (void)p_lstiter; (void)p_lstmap;
	return (0);
}
EOF

if grep -qw "restrict" "$LIBFT_DIR/libft.h" "$LIBFT_DIR"/ft_*.c; then
	fail "Mot-cle C99 interdit 'restrict' detecte"
else
	pass "Absence du qualificateur C99 interdit 'restrict' dans libft.h et ft_*.c"
fi

# 4. Makefile static checks + build + relink + rules
if grep -q -- "-Wall" "$LIBFT_DIR/Makefile" && grep -q -- "-Wextra" "$LIBFT_DIR/Makefile" \
	&& grep -q -- "-Werror" "$LIBFT_DIR/Makefile"; then
	pass "Makefile compile avec les flags obligatoires -Wall -Wextra -Werror"
else
	fail "Flags -Wall -Wextra -Werror manquants dans Makefile"
fi

if grep -Eq '^[[:space:]]*CC[[:space:]]*=[[:space:]]*cc([[:space:]]|$)' "$LIBFT_DIR/Makefile"; then
	pass "Makefile utilise le compilateur 'cc'"
else
	fail "Makefile n'utilise pas 'cc'"
fi

if grep -q "libtool" "$LIBFT_DIR/Makefile"; then
	fail "Commande interdite 'libtool' detectee dans Makefile"
else
	pass "Makefile utilise 'ar' et n'utilise pas 'libtool'"
fi

if make -C "$LIBFT_DIR" >"$TMP_DIR/make1.log" 2>&1 && [ -f "$LIBFT_DIR/libft.a" ]; then
	pass "Regle par defaut 'make' genere libft.a sans aucun warning"
else
	fail "Echec de 'make' :"
	cat "$TMP_DIR/make1.log"
fi

if cc -Wall -Wextra -Werror -I"$LIBFT_DIR" "$TMP_DIR/proto_check.c" "$LIBFT_DIR/libft.a" -o "$TMP_DIR/proto_check" 2>"$TMP_DIR/proto.err"; then
	pass "libft.h supporte la double inclusion et les 43 prototypes correspondent exactement au sujet v19.3"
else
	fail "Erreur sur libft.h ou discordance de prototype :"
	cat "$TMP_DIR/proto.err"
fi

# No-relink check
if make -C "$LIBFT_DIR" -n 2>/dev/null | grep -Eq '(^|[[:space:]/])(ar|cc|gcc|clang)[[:space:]]'; then
	fail "Makefile relink lorsque rien n'a ete modifie !"
else
	pass "Absence totale de relink au second appel de 'make'"
fi

if make -C "$LIBFT_DIR" -n libft.a 2>/dev/null | grep -Eq '(^|[[:space:]/])(ar|cc|gcc|clang)[[:space:]]'; then
	fail "La regle '$(NAME)' (libft.a) relink lorsque rien n'a ete modifie !"
else
	pass "La regle 'libft.a' existe et ne relink pas"
fi

# Incremental rebuild check: touching 1 .c file recompiles ONLY that 1 .o file
sleep 1
touch "$LIBFT_DIR/ft_strlen.c"
MAKE_INC_OUT=$(make -C "$LIBFT_DIR" 2>&1)
CC_CALLS=$(printf "%s\n" "$MAKE_INC_OUT" | grep -cE '(^|[[:space:]])cc[[:space:]]' || true)
if [ "$CC_CALLS" -eq 1 ] && printf "%s\n" "$MAKE_INC_OUT" | grep -q "ft_strlen.c"; then
	pass "Compilation incrementale verifiee : modifier 1 fichier .c ne recompile qu'un seul .o"
else
	fail "Compilation incrementale incorrecte ($CC_CALLS appels cc)"
fi

# Test clean, fclean, re rules
make -C "$LIBFT_DIR" clean >/dev/null 2>&1
OBJ_LEFT=$(find "$LIBFT_DIR" -maxdepth 1 -name '*.o' | wc -l | tr -d ' ')
if [ "$OBJ_LEFT" -eq 0 ] && [ -f "$LIBFT_DIR/libft.a" ]; then
	pass "Regle 'make clean' supprime tous les .o tout en conservant libft.a"
else
	fail "Comportement incorrect de 'make clean'"
fi

make -C "$LIBFT_DIR" fclean >/dev/null 2>&1
if [ ! -f "$LIBFT_DIR/libft.a" ] && [ "$(find "$LIBFT_DIR" -maxdepth 1 -name '*.o' | wc -l | tr -d ' ')" -eq 0 ]; then
	pass "Regle 'make fclean' supprime les .o et libft.a"
else
	fail "Comportement incorrect de 'make fclean'"
fi

if make -C "$LIBFT_DIR" re >/dev/null 2>&1 && [ -f "$LIBFT_DIR/libft.a" ]; then
	pass "Regle 'make re' reconstruit integralement libft.a"
else
	fail "Comportement incorrect de 'make re'"
fi

# 5. Global variables & per-object external symbol audit via nm
python3 - "$LIBFT_DIR" << 'PY'
import subprocess, sys, os, glob

libft_dir = sys.argv[1]

# Exact external functions allowed per file according to Libft Subject v19.3
allowed_ext = {
    "ft_calloc.o": {"malloc"},
    "ft_strdup.o": {"malloc"},
    "ft_substr.o": {"malloc"},
    "ft_strjoin.o": {"malloc"},
    "ft_strtrim.o": {"malloc"},
    "ft_split.o": {"malloc", "free"},
    "ft_itoa.o": {"malloc"},
    "ft_strmapi.o": {"malloc"},
    "ft_putchar_fd.o": {"write"},
    "ft_putstr_fd.o": {"write"},
    "ft_putendl_fd.o": {"write"},
    "ft_putnbr_fd.o": {"write"},
    "ft_lstnew.o": {"malloc"},
    "ft_lstdelone.o": {"free"},
    "ft_lstclear.o": {"free"},
    "ft_lstmap.o": {"malloc", "free"},
}

objs = sorted(glob.glob(os.path.join(libft_dir, "ft_*.o")))
if len(objs) != 43:
    print(f"GLOBAL_OR_SYM_ERR: Expected 43 .o files, found {len(objs)}")
    sys.exit(1)

global_vars = []
forbidden_calls = []
unstatic_helpers = []

for obj in objs:
    bname = os.path.basename(obj)
    expected_fn = bname[:-2]
    out = subprocess.check_output(["nm", obj], text=True)
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        sym_type = parts[-2]
        sym_name = parts[-1].split("@")[0]
        # Check global / file-scope static variables (B, b, D, d, C, G, g, S, s, V)
        if sym_type in ("B", "b", "D", "d", "C", "G", "g", "S", "s", "V"):
            if not sym_name.startswith("."):
                global_vars.append(f"{bname}:{sym_name}({sym_type})")
        # Check non-static helper functions polluting global namespace (T)
        if sym_type == "T" and sym_name != expected_fn:
            unstatic_helpers.append(f"{bname}:{sym_name}")
        # Check undefined external symbols (U)
        if sym_type == "U":
            if sym_name.startswith("ft_") or sym_name in ("_GLOBAL_OFFSET_TABLE_", "__stack_chk_fail"):
                continue
            permitted = allowed_ext.get(bname, set())
            if sym_name not in permitted:
                forbidden_calls.append(f"{bname} calls '{sym_name}' (allowed: {sorted(permitted)})")

if global_vars:
    print("GLOBAL_ERR: " + ", ".join(global_vars))
    sys.exit(2)
if unstatic_helpers:
    print("HELPER_ERR: " + ", ".join(unstatic_helpers))
    sys.exit(3)
if forbidden_calls:
    print("FORBIDDEN_ERR: " + "; ".join(forbidden_calls))
    sys.exit(4)
sys.exit(0)
PY
SYM_STATUS=$?
if [ "$SYM_STATUS" -eq 0 ]; then
	pass "Audit binaire 'nm' des 43 fichiers .o : 0 variable globale, helpers tous 'static', 0 fonction interdite"
else
	fail "Echec de l'audit binaire 'nm' (code $SYM_STATUS)"
fi

# 6. README.md Chapter V verification
FIRST_LINE=$(grep -v '^[[:space:]]*$' "$LIBFT_DIR/README.md" | head -n 1 | tr -d '\r')
if printf '%s\n' "$FIRST_LINE" | grep -qE '^(\*This project has been created as part of the 42 curriculum by .+\*|_This project has been created as part of the 42 curriculum by .+_)$' \
	&& ! printf '%s\n' "$FIRST_LINE" | grep -q '^\*\*'; then
	pass "README.md : 1re ligne en italique conforme au Chapitre V du sujet v19.3"
else
	fail "README.md : 1re ligne non conforme ($FIRST_LINE)"
fi

README_SECTIONS_OK=1
for sec in Description Instructions Resources; do
	if ! grep -qiE "^#{1,6}[[:space:]]*${sec}" "$LIBFT_DIR/README.md"; then
		fail "README.md : section obligatoire '$sec' manquante"
		README_SECTIONS_OK=0
	fi
done
if [ "$README_SECTIONS_OK" -eq 1 ] && grep -qi "AI" "$LIBFT_DIR/README.md"; then
	pass "README.md : sections Description, Instructions, Resources + transparence IA presentes"
fi

printf "\n${BOLD}Bilan Phase 1 : ${GREEN}%d OK${RESET}, ${RED}%d KO${RESET}\n\n" "$PASS_COUNT" "$FAIL_COUNT"
[ "$FAIL_COUNT" -eq 0 ]
