LIBFT_DIR	= ..
LIBFT_A		= $(LIBFT_DIR)/libft.a
LIBFT_SRCS	= $(wildcard $(LIBFT_DIR)/ft_*.c)

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -g -I$(LIBFT_DIR)
ASAN_FLAGS	= -fsanitize=address,undefined -fno-omit-frame-pointer -O1
WRAP_FLAGS	= -Wl,--wrap=malloc -Wl,--wrap=free

TEST_SRCS	= test_main.c test_part1.c test_part2.c test_part3.c

BIN_UNIT	= bin_test_unit
BIN_ASAN	= bin_test_asan
BIN_MALLOC	= bin_test_malloc_fail

all: test

$(LIBFT_A):
	$(MAKE) -C $(LIBFT_DIR)

$(BIN_UNIT): $(TEST_SRCS) test_framework.h $(LIBFT_A)
	$(CC) $(CFLAGS) $(TEST_SRCS) $(LIBFT_A) -o $(BIN_UNIT)

$(BIN_ASAN): $(TEST_SRCS) test_framework.h $(LIBFT_SRCS)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) $(TEST_SRCS) $(LIBFT_SRCS) -o $(BIN_ASAN)

$(BIN_MALLOC): test_malloc_fail.c $(LIBFT_A)
	$(CC) $(CFLAGS) test_malloc_fail.c $(LIBFT_A) $(WRAP_FLAGS) -o $(BIN_MALLOC)

setup:
	./test_setup.sh

unit: $(BIN_UNIT)
	./$(BIN_UNIT) "COMPILATION STANDARD"

asan: $(BIN_ASAN)
	ASAN_OPTIONS="detect_leaks=1:allocator_may_return_null=1:halt_on_error=1" \
	UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1" \
	./$(BIN_ASAN) "ASAN + UBSAN + LSAN"

alloc_fail: $(BIN_MALLOC)
	./$(BIN_MALLOC)

test:
	./run_all_tests.sh

clean:
	rm -f *.o

fclean: clean
	rm -f $(BIN_UNIT) $(BIN_ASAN) $(BIN_MALLOC)

re: fclean all

.PHONY: all setup unit asan alloc_fail test clean fclean re
