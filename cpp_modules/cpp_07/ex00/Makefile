CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: whatever

whatever: main.o
	$(CXX) $(CXXFLAGS) -o whatever main.o

main.o: main.cpp whatever.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	rm -f *.o

fclean: clean
	rm -f whatever

re: fclean all

.PHONY: all clean fclean re
