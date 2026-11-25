CC       = gcc
CFLAGS   = -Wall -Wextra -Werror
NAME     = bsq
SRC      = bsq.c

all: $(NAME)

$(NAME): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f $(NAME)

.PHONY: all clean
