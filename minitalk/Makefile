CC       = cc
CFLAGS   = -Wall -Wextra -Werror
RM       = rm -f

NAME     = client server
BONUS    = client_bonus server_bonus
OBJ      = utils.o transfer.o

all: $(NAME)

client: client.o $(OBJ)
	$(CC) $(CFLAGS) client.o $(OBJ) -o client

server: server.o $(OBJ)
	$(CC) $(CFLAGS) server.o $(OBJ) -o server

bonus: $(BONUS)

client_bonus: client_bonus.o $(OBJ)
	$(CC) $(CFLAGS) client_bonus.o $(OBJ) -o client_bonus

server_bonus: server_bonus.o $(OBJ)
	$(CC) $(CFLAGS) server_bonus.o $(OBJ) -o server_bonus

%.o: %.c minitalk.h
	$(CC) $(CFLAGS) -c $< -o $@

client_bonus.o: minitalk_bonus.h
server_bonus.o: minitalk_bonus.h

clean:
	$(RM) $(OBJ) client.o server.o client_bonus.o server_bonus.o

fclean: clean
	$(RM) $(NAME) $(BONUS)

re: fclean all

.PHONY: all bonus clean fclean re
