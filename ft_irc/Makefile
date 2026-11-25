# ============================================================================ #
#  ft_irc — Makefile (42 School)                                             #
# ============================================================================ #

NAME        := ircserv
CC          := g++
CXXFLAGS    := -std=c++98 -Wall -Wextra -Werror -O2
INCLUDES    := -Iincludes
SRC_DIR     := srcs
OBJ_DIR     := objs

SRCS        := $(SRC_DIR)/main.cpp \
               $(SRC_DIR)/Server.cpp \
               $(SRC_DIR)/Client.cpp \
               $(SRC_DIR)/Channel.cpp \
               $(SRC_DIR)/Utils.cpp \
               $(SRC_DIR)/commands/Commands.cpp

OBJS        := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR) $(OBJ_DIR)/commands
	$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(NAME)

re: clean all

fclean: clean

.PHONY: all clean re fclean
