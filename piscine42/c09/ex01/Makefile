# Makefile C09 ex01 : compile libft.a depuis srcs/ avec les headers
# de includes/. Les .o sont generes a cote de leurs .c.
NAME	= libft.a
CC		= cc
CFLAGS	= -Wall -Wextra -Werror
RM		= rm -f
SRCS	= $(wildcard srcs/*.c)
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
