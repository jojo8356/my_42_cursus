CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: iter

iter: main.o
	$(CXX) $(CXXFLAGS) -o iter main.o

main.o: main.cpp iter.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	rm -f *.o

fclean: clean
	rm -f iter

re: fclean all

.PHONY: all clean fclean re
