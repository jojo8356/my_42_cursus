# ──────────────────────────────────────────────────────────────────────
# fract-ol — Fractal renderer (42, Rank 02, Graphic branch)
# ──────────────────────────────────────────────────────────────────────

# ---- compiler & flags ----
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g -fno-omit-frame-pointer
CFLAGS  += -I includes -I ../libft -I ../mlx
LDFLAGS = -lXext -lX11 -lm -lpthread

# ---- paths ----
LIBFT   = ../libft/libft.a
MLX_A   = ../mlx/libmlx.a

# ---- sources ----
SRCS = src/main.c \
       src/args.c \
       src/init.c \
       src/hooks.c \
       src/render.c \
       src/render_core.c \
       src/fractal_math.c \
       src/color.c \
       src/transform.c

OBJS = $(SRCS:.c=.o)

# ---- targets ----
NAME    = fractol

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT) $(MLX_A)
	$(CC) $(OBJS) $(LIBFT) $(MLX_A) $(LDFLAGS) -o $(NAME)

%.o: %.c includes/fract-ol.h
	$(CC) $(CFLAGS) -c $< -o $@

# ---- library build (delegated) ----
$(LIBFT):
	$(MAKE) -C ../libft

$(MLX_A):
	$(MAKE) -C ../mlx

# ---- run shortcuts ----
m: $(NAME)
	./$(NAME) mandelbrot

j: $(NAME)
	./$(NAME) julia -0.8 0.156

s: $(NAME)
	./$(NAME) burning_ship

t: $(NAME)
	./$(NAME) tricorn

p: $(NAME)
	./$(NAME) phoenix 0.5667 0.0 -0.5

mb: $(NAME)
	./$(NAME) multibrot 3

# ---- tests ----
test: $(NAME)
	./tests/run_tests.sh

# ---- clean ----
clean:
	rm -f $(OBJS) $(NAME)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all m j s t p mb test clean fclean re
