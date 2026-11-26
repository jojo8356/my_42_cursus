NAME := codexion
CC := gcc
CFLAGS := -Wall -Wextra -Werror -pthread -I. -Iutils -Ilog -Idongle -Icoder -Imonitor
SRCS := codexion.c \
	utils/utils.c \
	log/log.c \
	dongle/heap.c \
	dongle/dongle.c \
	coder/coder.c \
	monitor/monitor.c
OBJS := $(SRCS:.c=.o)
HEADERS := codexion.h utils/utils.h log/log.h dongle/heap.h dongle/dongle.h \
	coder/coder.h monitor/monitor.h

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

all: $(NAME)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean:
	rm -f $(OBJS) $(NAME)

re: fclean all
