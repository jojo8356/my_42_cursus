# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/08 21:00:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/08 21:00:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= push_swap
BONUS	= checker

CC		= cc
CFLAGS	= -Wall -Wextra -Werror

LIBFT	= libft/libft.a

SRC		= src/main.c \
		  src/ps_parse.c \
		  src/ps_parse_utils.c \
		  src/st_basic.c \
		  src/st_ops.c \
		  src/do_ops.c \
		  src/do_ops_rot.c \
		  src/do_ops_rrot.c \
		  src/exec_instr.c \
		  src/op_buffer.c \
		  src/ps_rank.c \
		  src/sort_small.c \
		  src/sort_small2.c \
		  src/sort_chunk.c \
		  src/sort_cost.c \
		  src/sort_insert.c \
		  src/sort_adaptive.c

SRC_B	= src/bonus/checker_main.c \
		  src/bonus/checker_read.c

OBJ		= $(SRC:src/%.c=obj/%.o)
OBJ_B	= $(SRC_B:src/%.c=obj/%.o)

# The bonus objects are compiled a second time on purpose: the shared sources
# must not drag a main() from the mandatory binary into the checker.
SHARED	= $(filter-out src/main.c,$(SRC))
OBJ_S	= $(SHARED:src/%.c=obj/bonus/%.o)

all: $(NAME)

$(NAME): $(OBJ) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJ) $(LIBFT) -o $(NAME)
	@echo "$(NAME): mandatory build"

bonus: $(BONUS)

$(BONUS): $(OBJ_B) $(OBJ_S) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJ_B) $(OBJ_S) $(LIBFT) -o $(BONUS)
	@echo "$(BONUS): bonus build"

$(LIBFT):
	$(MAKE) -C libft

obj/%.o: src/%.c include/push_swap.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I include -I libft -c $< -o $@

obj/bonus/%.o: src/%.c include/push_swap.h include/checker.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I include -I libft -c $< -o $@

clean:
	$(MAKE) -C libft clean
	rm -rf obj

fclean: clean
	rm -f $(NAME) $(BONUS)

re: fclean all

.PHONY: all bonus clean fclean re
