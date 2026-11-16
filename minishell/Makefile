# ****************************************************************************
#                                                                       #
#                                                        :::      ::::::::   #
#   Makefile                                              :+:      :+:    :+:   #
#                                                    +:+ +:+         +:+      #
#   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        #
#                                                +#+#+#+#+#+   +#+           #
#   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             #
#   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr      #
#                                                                       #
# ****************************************************************************

NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror
INC = -I includes -I libft

SRCS = src/0_main.c \
    src/1_tokenizer.c \
    src/1_tokenizer_word.c \
    src/1_tokenizer_op.c \
    src/1_tokenizer_main.c \
    src/2_parser_utils.c \
    src/2_parser_argv.c \
    src/2_parser_main.c \
    src/3_expander_var.c \
    src/3_expander_quotes.c \
    src/3_expander_wildcard.c \
    src/3_expander_main.c \
    src/4_executor.c \
    src/4_executor_main.c \
    src/4_executor_dispatch.c \
    src/5_builtins.c \
    src/5_builtin_echo.c \
    src/5_builtin_cd.c \
    src/5_builtin_pwd.c \
    src/5_builtin_env.c \
    src/5_builtin_exit.c \
    src/6_redirections.c \
    src/7_signals.c \
    src/8_cleanup.c \
    src/shell_state.c \
    src/readline_mock.c

OBJS = $(SRCS:.c=.o)
LIBFT = libft/libft.a

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $(NAME)

$(LIBFT):
	$(MAKE) -C libft

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJS)
	$(MAKE) -C libft clean

fclean: clean
	rm -f $(NAME)

re: fclean all

bonus: all

.PHONY: all clean fclean re bonus
