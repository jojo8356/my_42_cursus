# *****************************************************************************
#
#   Makefile cub3d
#
# *****************************************************************************

NAME        = cub3D
BONUS_NAME    = cub3D_bonus

CC            = cc
CFLAGS        = -Wall -Wextra -Werror -I include -I libft
LDLIBS        = libft/libft.a -lm

# miniLibX système (réel). Sur une machine sans X11/miniLibX, le build
# réel échoue ; utiliser `make test` pour le build headless (shim).
MLX_LIBS    = -lmlx -lXext -lX11

SRCS        = src/main.c src/parser.c src/textures.c src/player.c \
                src/raycasting.c src/render.c src/hooks.c src/utils.c
OBJS        = $(SRCS:.c=.o)

# ---------------------------------------------------------------- tests
TEST_BIN    = tests/cub3d_test
TEST_SRCS    = $(SRCS) tests/shim/mlx_shim.c
TEST_OBJS    = $(TEST_SRCS:.c=.o)

all:        $(NAME)

$(NAME):    $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDLIBS) $(MLX_LIBS)

# le binaire bonus est le même code ; le bonus est activé par le nom
# (main.c détecte "bonus" dans argv[0]). On crée un lien symbolique.
bonus:        $(NAME)
	ln -sf $(NAME) $(BONUS_NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# build headless avec le shim miniLibX (pour les tests)
tests/shim/mlx_shim.o: tests/shim/mlx_shim.c
	$(CC) $(CFLAGS) -I tests/shim -c $< -o $@

$(TEST_BIN): $(TEST_OBJS)
	$(CC) $(CFLAGS) -I tests/shim -o $@ $(TEST_OBJS) $(LDLIBS)

test:
	bash tests/run_tests.sh

clean:
	rm -f $(OBJS) $(TEST_OBJS)

fclean:    clean
	rm -f $(NAME) $(BONUS_NAME) $(TEST_BIN)

re:            fclean all

re_b:        fclean bonus

.PHONY: all bonus clean fclean re re_b test
