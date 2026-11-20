NAME = PmergeMe
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): main.o PmergeMe.o
	$(CXX) $(CXXFLAGS) -o $(NAME) main.o PmergeMe.o

main.o: main.cpp PmergeMe.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

PmergeMe.o: PmergeMe.cpp PmergeMe.hpp
	$(CXX) $(CXXFLAGS) -c PmergeMe.cpp -o PmergeMe.o

clean:
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
