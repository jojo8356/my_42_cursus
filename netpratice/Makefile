# **************************************************************************
#                                                                          #
#   Makefile                         :+:      :+:    :+:   #
#                                                    +:+ +:+         +:+     #
#   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        #
#                                                +#+#+#+#+#+   +#+           #
#   NetPratice — build + tests.                                          #
# **************************************************************************

NAME        = netpratice
CC        = gcc
CFLAGS        = -Wall -Wextra -Werror -O2 -Iinclude -Ilibft
LDFLAGS        = -Llibft -lft

SRC_DIR    = src
INC_DIR    = include
LIBFT_DIR    = libft
TESTS_DIR    = tests

SRCS        = $(SRC_DIR)/ip.c $(SRC_DIR)/mask.c $(SRC_DIR)/routing.c \
              $(SRC_DIR)/json.c $(SRC_DIR)/solution.c $(SRC_DIR)/helpers.c \
              $(SRC_DIR)/validate.c $(SRC_DIR)/level.c $(SRC_DIR)/solver.c \
              $(SRC_DIR)/main.c

# sources sans main.c (pour les tests)
SRCS_CORE   = $(filter-out $(SRC_DIR)/main.c, $(SRCS))

OBJS        = $(SRCS:.c=.o)

TEST_SRCS    = $(wildcard $(TESTS_DIR)/*.c)
TEST_BINS    = $(TEST_SRCS:.c=)

all: $(NAME)

$(NAME): $(OBJS) libft/libft.a
	$(CC) $(OBJS) -o $(NAME) $(LDFLAGS)

libft/libft.a:
	$(MAKE) -C libft

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/netpratice.h libft/libft.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BINS)
	@echo "=== NetPratice tests ==="
	@fail=0; \
	for t in $(TEST_BINS); do \
	  echo "--- $$t ---"; \
	  ./$$t || fail=1; \
	done; \
	if [ $$fail -eq 0 ]; then echo "ALL TESTS PASSED"; else echo "SOME TESTS FAILED"; exit 1; fi

$(TESTS_DIR)/%: $(TESTS_DIR)/%.c $(SRCS_CORE) libft/libft.a
	$(CC) $(CFLAGS) $< $(SRCS_CORE) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(NAME)
	rm -f $(TEST_BINS)
	$(MAKE) -C libft clean 2>/dev/null || true

.PHONY: all test clean
