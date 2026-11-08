# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/04 08:00:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/08 19:00:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= rush-02
CC		= cc
CFLAGS	= -Wall -Wextra -Werror
SRC		= main.c dict.c convert.c convert_group.c ft_utils.c
OBJ		= $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.c rush02.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
