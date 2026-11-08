*This project has been created as part of the 42 curriculum by student42.*

# Libft — Your Very First Own Library

## Description

**Libft** is the first foundational project of the **42 Common Core** curriculum (Subject **v19.3**, Norm **v4.1**). Its goal is to build from scratch a standalone, reusable static C library named **`libft.a`** containing re-implementations of essential standard C library (`libc`) routines alongside custom string, memory, file-descriptor output, and singly linked-list utilities.

All **43 mandatory functions** strictly follow the 42 Norm, use zero global variables, compile cleanly with `cc -Wall -Wextra -Werror`, and rely exclusively on the external functions permitted by the subject (`malloc`, `free`, and `write`).

### Library Contents (43 Functions)

#### Part 1 — Libc Functions (23 functions)
- **Character classification & case conversion**: `ft_isalpha`, `ft_isdigit`, `ft_isalnum`, `ft_isascii`, `ft_isprint` (returning exact `1` or `0` per v19.3), `ft_toupper`, `ft_tolower`.
- **Memory operations**: `ft_memset`, `ft_bzero`, `ft_memcpy`, `ft_memmove` (overlap-safe), `ft_memchr`, `ft_memcmp`, `ft_calloc` (overflow-checked and returning a freeable pointer on zero size).
- **String operations & conversion**: `ft_strlen`, `ft_strlcpy`, `ft_strlcat`, `ft_strchr`, `ft_strrchr`, `ft_strncmp`, `ft_strnstr`, `ft_strdup`, `ft_atoi`.

#### Part 2 — Additional Functions (11 functions)
- **String allocation & functional iteration**: `ft_substr`, `ft_strjoin`, `ft_strtrim`, `ft_split`, `ft_itoa`, `ft_strmapi`, `ft_striteri`.
- **File-descriptor output**: `ft_putchar_fd`, `ft_putstr_fd`, `ft_putendl_fd`, `ft_putnbr_fd`.

#### Part 3 — Singly Linked Lists (`t_list`, 9 functions)
- **Node & list management**: `ft_lstnew`, `ft_lstadd_front`, `ft_lstsize` (returning `unsigned int` per v19.3), `ft_lstlast`, `ft_lstadd_back`, `ft_lstdelone`, `ft_lstclear`, `ft_lstiter`, `ft_lstmap`.

## Instructions

### Prerequisites
- POSIX-compliant environment (Linux / macOS)
- `cc` compiler (GCC or Clang), `make`, and `ar`

### Compilation

Run `make` at the root of the repository to build `libft.a`:

```sh
make          # Compiles all 43 object files and archives libft.a
make clean    # Removes intermediate .o object files
make fclean   # Removes .o object files and libft.a
make re       # Performs a full clean rebuild (fclean + all)
```

### Usage in Another C Project

Include `libft.h` in your source code and link against `libft.a`:

```sh
cc -Wall -Wextra -Werror main.c -L. -lft -o my_program
./my_program
```

## Resources

### Classic References
- Linux Programmer's Manual (`man 3 strlen`, `man 3 memmove`, `man 3 calloc`, `man 3 strlcpy`, `man 3 strlcat`): https://man7.org/linux/man-pages/
- ISO/IEC 9899 C Standard Library documentation: https://en.cppreference.com/w/c
- GNU Make Manual: https://www.gnu.org/software/make/manual/
- Official 42 **Libft Subject (Version 19.3)** and **42 Norm (Version 4.1)**
- Official 42 Norminette static style checker: https://github.com/42School/norminette

### AI Usage
In accordance with Chapter III (*AI Instructions*) and Chapter V of the 42 curriculum subject:
- **Subject analysis & version verification**: Identifying the exact specifications introduced in Libft v19.2 and v19.3 (integration of Part 3 linked lists into mandatory `make`, exact `0`/`1` return values for `ft_is*`, `unsigned int` return type for `ft_lstsize`, and `ft_calloc` zero-size behavior).
- **Code implementation & edge-case verification**: Writing norm-compliant implementations and designing external unit, sanitizer, and allocation-failure tests across all 43 functions.
- **Documentation**: Structuring and formatting this `README.md`.
