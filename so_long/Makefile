# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/08 22:30:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/08 22:30:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= so_long
BONUS	= so_long_bonus

CC		= cc
CFLAGS	= -Wall -Wextra -Werror

LIBFT	= libft/libft.a

# The minilibX is used as a system library when one is installed (the 42
# clusters ship it); MLX_DIR lets a checkout build it from sources instead.
MLX_DIR	?= mlx
MLX_INC	?=
MLX_LIB	?= -lmlx -lXext -lX11 -lm

SRC		= src/main.c \
		  src/map_load.c \
		  src/map_valid.c \
		  src/map_sprites.c \
		  src/map_utils.c \
		  src/read_file.c \
		  src/render.c \
		  src/render_load.c \
		  src/game.c \
		  src/game_hooks.c \
		  src/bonus_stub.c

SRC_B	= src/main_bonus.c \
		  src/map_load.c \
		  src/map_valid.c \
		  src/map_sprites.c \
		  src/map_utils.c \
		  src/read_file.c \
		  src/render.c \
		  src/render_load.c \
		  src/game.c \
		  src/game_hooks.c \
		  src/bonus/enemy_bonus.c \
		  src/bonus/render_bonus.c \
		  src/bonus/bonus_hook_bonus.c \
		  src/bonus/charset_bonus.c

# Separate object trees. Sharing obj/ would be a time bomb: make all && make
# bonus would relink the bonus from a main.o that contains the mandatory main().
OBJ		= $(SRC:src/%.c=obj/%.o)
OBJ_B	= $(SRC_B:src/%.c=obj/bonus/%.o)

all: $(NAME)

$(NAME): $(OBJ) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJ) $(LIBFT) $(MLX_LIB) -o $(NAME)
	@echo "$(NAME): mandatory build"

bonus: $(BONUS)

$(BONUS): $(OBJ_B) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJ_B) $(LIBFT) $(MLX_LIB) -o $(BONUS)
	@echo "$(BONUS): bonus build"

$(LIBFT):
	$(MAKE) -C libft

obj/bonus/%.o: src/%.c include/so_long.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I include -I libft $(MLX_INC) -c $< -o $@

obj/%.o: src/%.c include/so_long.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I include -I libft $(MLX_INC) -c $< -o $@

clean:
	$(MAKE) -C libft clean
	rm -rf obj

fclean: clean
	rm -f $(NAME) $(BONUS)

re: fclean all

# Used by tests/build.sh: prints the exact source lists so the test build can
# never drift from the real one.
print-sources:
	@echo "MANDATORY $(SRC)"
	@echo "BONUS $(SRC_B)"

.PHONY: all bonus clean fclean re print-sources
