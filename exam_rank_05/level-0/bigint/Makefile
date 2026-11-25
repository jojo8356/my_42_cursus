# bigint — 42 Exam Rank 05 (level 0)
CXX       = g++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98
TARGET    = bigint
SRCS      = bigint.cpp main.cpp

all: $(TARGET)

$(TARGET): $(SRCS) bigint.hpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
