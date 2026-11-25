# vect2 — 42 Exam Rank 05 (level 0)
CXX       = g++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98
TARGET    = vect2
SRCS      = vect2.cpp main.cpp

all: $(TARGET)

$(TARGET): $(SRCS) vect2.hpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
