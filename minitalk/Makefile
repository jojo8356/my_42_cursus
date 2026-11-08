# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/28 10:00:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/08 18:30:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= client server
BONUS			= client_bonus server_bonus

CC				= cc
CFLAGS			= -Wall -Wextra -Werror
CPPFLAGS		= -I$(INC_DIR)
RM				= rm -f

SRC_DIR			= src
INC_DIR			= include
OBJ_DIR			= obj

COMMON_SRCS		= utils.c transfer.c
COMMON_OBJS		= $(addprefix $(OBJ_DIR)/, $(COMMON_SRCS:.c=.o))

CLIENT_OBJ		= $(OBJ_DIR)/client.o
SERVER_OBJ		= $(OBJ_DIR)/server.o
CLIENT_B_OBJ	= $(OBJ_DIR)/client_bonus.o
SERVER_B_OBJ	= $(OBJ_DIR)/server_bonus.o

all: $(NAME)

client: $(CLIENT_OBJ) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) $(COMMON_OBJS) -o client

server: $(SERVER_OBJ) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJ) $(COMMON_OBJS) -o server

bonus: $(BONUS)

client_bonus: $(CLIENT_B_OBJ) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_B_OBJ) $(COMMON_OBJS) -o client_bonus

server_bonus: $(SERVER_B_OBJ) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(SERVER_B_OBJ) $(COMMON_OBJS) -o server_bonus

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/minitalk.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(CLIENT_B_OBJ): $(INC_DIR)/minitalk_bonus.h
$(SERVER_B_OBJ): $(INC_DIR)/minitalk_bonus.h

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME) $(BONUS)

re: fclean all

.PHONY: all bonus clean fclean re
