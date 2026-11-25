CC       = gcc
CFLAGS   = -Wall -Wextra -Werror
NAME     = life
SRC      = life.c

all: $(NAME)

$(NAME): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f $(NAME)

.PHONY: all clean
