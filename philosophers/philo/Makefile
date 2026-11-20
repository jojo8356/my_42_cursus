NAME = philo
SRC = philo.c utils.c philosopher.c monitor.c
OBJ = $(SRC:.c=.o)
CC = cc
CFLAGS = -Wall -Wextra -Werror
LDLIBS = -lpthread
HDR = philo.h

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LDLIBS)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

bonus:
	@$(MAKE) --no-print-directory -C ../philo_bonus all

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean
	@$(MAKE) --no-print-directory all

.PHONY: all bonus clean fclean re
