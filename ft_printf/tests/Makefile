NAME	= test_ft_printf
CC		= cc
CFLAGS	= -Wall -Wextra -Werror -Wno-format
LIB		= ../libftprintf.a

all: $(NAME)

cases.h: generate_tests.py
	python3 generate_tests.py

$(NAME): test_ft_printf.c cases.h $(LIB)
	$(CC) $(CFLAGS) -I../include test_ft_printf.c -L.. -lftprintf -o $(NAME)

$(LIB):
	$(MAKE) -C .. bonus

run: $(NAME)
	./$(NAME)

valgrind: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=99 -q ./$(NAME)

sanitize: cases.h
	$(MAKE) -C .. fclean
	$(MAKE) -C .. bonus CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address,undefined"
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined -I../include test_ft_printf.c -L.. -lftprintf -o test_asan
	./test_asan
	$(MAKE) -C .. fclean
	$(MAKE) -C .. bonus

clean:
	rm -f $(NAME) test_asan

fclean: clean
	rm -f cases.h

re: fclean all

.PHONY: all run valgrind sanitize clean fclean re
