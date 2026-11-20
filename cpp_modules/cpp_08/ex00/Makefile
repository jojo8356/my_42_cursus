CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: easyfind

easyfind: main.o
	$(CXX) $(CXXFLAGS) -o easyfind main.o

main.o: main.cpp easyfind.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	rm -f *.o

fclean: clean
	rm -f easyfind

re: fclean all

.PHONY: all clean fclean re
