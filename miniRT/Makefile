# *****************************************************************************
#
#   Makefile miniRT
#
# *****************************************************************************

NAME        = miniRT

CC            = cc
CFLAGS        = -Wall -Wextra -Werror -I include -I libft
LDLIBS        = libft/libft.a -lm

# miniLibX système (réel). Sur une machine sans X11/miniLibX, le build
# réel échoue ; utiliser `make test` pour le build headless (shim).
MLX_LIBS    = -lmlx -lXext -lX11

SRCS        = src/main.c src/vector.c src/matrix.c src/utils.c src/objects.c \
                src/intersect.c src/lighting.c src/camera.c src/render.c \
                src/parser.c src/hooks.c src/mt.c src/texture.c
OBJS        = $(SRCS:.c=.o)

# ---------------------------------------------------------------- tests
TEST_BIN    = tests/minrt_test
TEST_SRCS    = src/vector.c src/matrix.c src/utils.c src/objects.c \
                src/intersect.c src/lighting.c src/camera.c src/render.c \
                src/parser.c src/hooks.c src/mt.c src/texture.c \
                tests/shim/mlx_shim.c
TEST_OBJS    = $(TEST_SRCS:.c=.o)

all:        $(NAME)

$(NAME):    $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDLIBS) $(MLX_LIBS)

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
	rm -f $(NAME) $(TEST_BIN)

re:            fclean all

.PHONY: all test clean fclean re
