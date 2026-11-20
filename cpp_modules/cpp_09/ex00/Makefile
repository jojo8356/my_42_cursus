NAME = btc
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): main.o BitcoinExchange.o
	$(CXX) $(CXXFLAGS) -o $(NAME) main.o BitcoinExchange.o

main.o: main.cpp BitcoinExchange.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

BitcoinExchange.o: BitcoinExchange.cpp BitcoinExchange.hpp
	$(CXX) $(CXXFLAGS) -c BitcoinExchange.cpp -o BitcoinExchange.o

clean:
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
