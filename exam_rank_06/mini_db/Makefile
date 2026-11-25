# mini_db — 42 Exam Rank 06
# Key-value database served over TCP (C++).

CXX       = g++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98
TARGET    = mini_db
SRCS      = mini_db.cpp

all: $(TARGET)

$(TARGET): $(SRCS) mini_db.hpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
