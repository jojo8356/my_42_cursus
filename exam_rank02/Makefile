# **************************************************************************** #
#                                                                              #
#                                                       :::      ::::::::     #
#    Makefile                                         :+:      :+:    :+:     #
#                                                    +:+ +:+         +:+      #
#   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                +#+#+#+#+#+   +#+            #
#    Created: 2026/11/12 21:00:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/12 21:00:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC      = cc
CFLAGS  = -Wall -Wextra -Werror
LDFLAGS =

LIBFT   = libft/libft.a

SRCS = main.c \
       level_1/level_1.c \
       level_2/level_2.c \
       level_3/level_3.c \
       level_4/level_4.c

OBJS = $(SRCS:.c=.o)

NAME = exam_rank02

INCLUDES = -I include -I libft

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) $(LDFLAGS) -o $(NAME)

main.o: main.c include/exam_rank02.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

level_1/level_1.o: level_1/level_1.c include/exam_rank02.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

level_2/level_2.o: level_2/level_2.c include/exam_rank02.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

level_3/level_3.o: level_3/level_3.c include/exam_rank02.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

level_4/level_4.o: level_4/level_4.c include/exam_rank02.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIBFT):
	$(MAKE) -C libft

# ---- tests ----
test: $(NAME)
	./tests/run_tests.sh

# ---- clean ----
clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all test clean fclean re
