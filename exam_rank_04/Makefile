# ************************************************************************** #
#  Exam Rank 04 — microshell                                               #
# ************************************************************************** #

NAME        = microshell
CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -O2
SRC_DIR     = .
OBJ_DIR     = obj

SRCS        = $(SRC_DIR)/parser.c $(SRC_DIR)/microshell.c
OBJS        = $(OBJ_DIR)/parser.o $(OBJ_DIR)/microshell.o

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c microshell.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ---- tests ----
# Les binaires de test linkent uniquement parser.o (microshell.o contient main)
TEST_SRCS   = $(wildcard tests/*.c)
TEST_BINS   = $(TEST_SRCS:.c=)

test: $(NAME) $(TEST_BINS)
	@echo "=== tests unitaires ==="
	@for t in $(TEST_BINS); do ./$$t || exit 1; done
	@echo "=== tests d'intégration ==="
	@bash tests/run_tests.sh

tests/%: tests/%.c $(OBJ_DIR)/parser.o microshell.h tests/test_framework.h
	$(CC) $(CFLAGS) -I. -Itests $< $(OBJ_DIR)/parser.o -o $@

clean:
	rm -rf $(OBJ_DIR) $(NAME) $(TEST_BINS)

.PHONY: all test clean
