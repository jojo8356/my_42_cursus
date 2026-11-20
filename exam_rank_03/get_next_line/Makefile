NAME = test_gnl
SRC = test_gnl.c get_next_line.c
OBJ = $(SRC:.c=.o)
CC = cc
CFLAGS = -Wall -Wextra -Werror
LDLIBS = -DBUFFER_SIZE=4242

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(LDLIBS) -o $(NAME) $(OBJ)

%.o: %.c get_next_line.h
	$(CC) $(CFLAGS) $(LDLIBS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean
	@$(MAKE) --no-print-directory all

.PHONY: all clean fclean re
