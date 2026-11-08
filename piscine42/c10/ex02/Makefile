NAME	= ft_tail
CC		= cc
CFLAGS	= -Wall -Wextra -Werror
RM		= rm -f
SRCS	= ft_tail.c tail_utils.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
