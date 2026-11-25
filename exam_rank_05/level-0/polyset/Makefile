# polyset — 42 Exam Rank 05 (level 0)
CXX       = g++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98
TARGET    = polyset
SRCS      = searchable_array_bag.cpp searchable_tree_bag.cpp set.cpp \
            given-code/array_bag.cpp given-code/tree_bag.cpp main.cpp
INCLUDES  = -I. -Igiven-code

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
