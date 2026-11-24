# **************************************************************************** #
#                                                                              #
#   Makefile principal — ft_containers (42 School)                            #
#   Orchestre : suite de test interne + tous les testeurs élèves.            #
#                                                                              #
#**************************************************************************** #

NAME            = ft_containers
CC             = g++
CXXFLAGS       = -Wall -Wextra -Werror -std=c++98
INCLUDES       = -Iincludes

# ---------------------------------------------------------------------- #
#  Suite de test interne (tests/test_containers.cpp)
# ---------------------------------------------------------------------- #
TEST_SRCS      = tests/test_containers.cpp
TEST_BIN       = tests/test_containers

# ---------------------------------------------------------------------- #
#  Testeurs élèves
# ---------------------------------------------------------------------- #
TESTERS        = llefranc monkey mazoise mli static

# ---------------------------------------------------------------------- #
#  Cibles
# ---------------------------------------------------------------------- #
.PHONY: all test testers clean fclean re

all: test testers

test: $(TEST_BIN)
	@echo "~ Run internal test suite ~"
	@$(TEST_BIN)

$(TEST_BIN): $(TEST_SRCS) includes/*.hpp
	@echo "~ Compiling internal test suite ~"
	@$(CC) $(CXXFLAGS) $(INCLUDES) $(TEST_SRCS) -o $(TEST_BIN)

testers:
	@for t in $(TESTERS); do \
		echo "~ Building tester: $$t ~"; \
		$(MAKE) -C tests/testers/tester_$$t re || exit 1; \
	done

clean:
	@rm -f $(TEST_BIN)
	@for t in $(TESTERS); do $(MAKE) -C tests/testers/tester_$$t clean || true; done

fclean: clean
	@rm -f $(TEST_BIN)
	@for t in $(TESTERS); do $(MAKE) -C tests/testers/tester_$$t fclean || true; done

re: fclean all
