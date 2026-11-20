NAME = RPN
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): main.o RPN.o
	$(CXX) $(CXXFLAGS) -o $(NAME) main.o RPN.o

main.o: main.cpp RPN.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

RPN.o: RPN.cpp RPN.hpp
	$(CXX) $(CXXFLAGS) -c RPN.cpp -o RPN.o

clean:
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
