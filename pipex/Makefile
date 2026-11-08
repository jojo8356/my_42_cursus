# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/08 12:00:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/08 12:00:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	pipex

LIBFT		=	libft/libft.a

CC			=	cc
CFLAGS		=	-Wall -Wextra -Werror
CPPFLAGS	=	-I include
RM			=	rm -rf

MANDATORY	=	src/main.c \
				src/pipex_error.c \
				src/pipex_split.c \
				src/pipex_path.c \
				src/pipex_exec.c \
				src/pipex_run.c

BONUS		=	src/main_bonus.c \
				src/pipex_error.c \
				src/pipex_split.c \
				src/pipex_path.c \
				src/pipex_exec.c \
				src/bonus/pipex_run_bonus.c \
				src/bonus/pipex_child_bonus.c \
				src/bonus/pipex_here_doc_bonus.c

OBJ_M		=	obj/mandatory
OBJ_B		=	obj/bonus

MANDATORY_OBJ	=	$(MANDATORY:src/%.c=$(OBJ_M)/%.o)
BONUS_OBJ		=	$(BONUS:src/%.c=$(OBJ_B)/%.o)

# Two distinct object trees on purpose: the two binaries share five sources but
# not the entry point, and mixing the objects of one flavour into the other is
# the classic way to ship a "bonus" binary that is silently mandatory. Each
# tree is rebuilt from scratch when the flavour changes, which also keeps the
# "must not relink" rule trivially true: `make` twice in a row does nothing the
# second time.

all:		$(NAME)

$(LIBFT):
	@$(MAKE) --no-print-directory -C libft

$(OBJ_M)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ_B)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ_B)/bonus/%.o: src/bonus/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(NAME):	$(LIBFT) $(MANDATORY_OBJ)
	$(CC) $(CFLAGS) $(MANDATORY_OBJ) $(LIBFT) -o $(NAME)
	@echo "$(NAME): mandatory build"

bonus:		$(LIBFT) $(BONUS_OBJ)
	$(CC) $(CFLAGS) $(BONUS_OBJ) $(LIBFT) -o $(NAME)
	@echo "$(NAME): bonus build"

clean:
	@$(MAKE) --no-print-directory -C libft clean
	$(RM) obj

fclean:		clean
	@$(MAKE) --no-print-directory -C libft fclean
	$(RM) $(NAME)

re:			fclean all

.PHONY:		all bonus clean fclean re
