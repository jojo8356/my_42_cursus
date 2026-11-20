CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: mutantstack

mutantstack: main.o
	$(CXX) $(CXXFLAGS) -o mutantstack main.o

main.o: main.cpp MutantStack.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	rm -f *.o

fclean: clean
	rm -f mutantstack

re: fclean all

.PHONY: all clean fclean re
