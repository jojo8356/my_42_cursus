# ──────────────────────────────────────────────────────────────────────
# fdf — Wireframe 3D terrain renderer (42, Rank 02, Graphic branch)
# ──────────────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g -fno-omit-frame-pointer
CFLAGS  += -I includes -I ../libft -I ../mlx
LDFLAGS = -lXext -lX11 -lm

LIBFT   = ../libft/libft.a
MLX_A   = ../mlx/libmlx.a

SRCS = src/main.c \
       src/parse.c \
       src/math.c \
       src/color.c \
       src/draw.c \
       src/render.c \
       src/hooks.c

OBJS = $(SRCS:.c=.o)

NAME = fdf

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT) $(MLX_A)
	$(CC) $(OBJS) $(LIBFT) $(MLX_A) $(LDFLAGS) -o $(NAME)

%.o: %.c includes/fdf.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBFT):
	$(MAKE) -C ../libft

$(MLX_A):
	$(MAKE) -C ../mlx

# ---- run shortcuts ----
run: $(NAME)
	./$(NAME) tests/maps/basic.fdf

# ---- tests ----
test: $(NAME)
	./tests/run_tests.sh

# ---- clean ----
clean:
	rm -f $(OBJS) $(NAME)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run test clean fclean re
