# mini_serv — 42 Exam Rank 06
# Non-blocking multi-client TCP chat server (C).

CC       = gcc
CFLAGS   = -Wall -Wextra -Werror -std=gnu11
TARGET   = mini_serv
SRC      = mini_serv.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
