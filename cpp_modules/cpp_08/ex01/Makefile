CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: span

span: main.o Span.o
	$(CXX) $(CXXFLAGS) -o span main.o Span.o

main.o: main.cpp Span.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

Span.o: Span.cpp Span.hpp
	$(CXX) $(CXXFLAGS) -c Span.cpp -o Span.o

clean:
	rm -f *.o

fclean: clean
	rm -f span

re: fclean all

.PHONY: all clean fclean re
