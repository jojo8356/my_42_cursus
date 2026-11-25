CXX       = g++
CXXFLAGS  = -std=c++98 -Wall -Wextra -Werror -O2
SRCDIR     = srcs
INCDIR     = includes
OBJDIR     = objs
TARGET     = webserv

SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/Server.cpp $(SRCDIR)/Request.cpp \
       $(SRCDIR)/Config.cpp $(SRCDIR)/Utils.cpp
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)

fclean: clean
	rm -rf $(OBJDIR)

re: fclean all

.PHONY: all clean fclean re
