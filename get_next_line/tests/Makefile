CC		= cc
CFLAGS	= -Wall -Wextra -Werror
SAN		= -g3 -fsanitize=address,undefined -fno-omit-frame-pointer

M_SRC	= ../get_next_line.c ../get_next_line_utils.c
B_SRC	= ../get_next_line_bonus.c ../get_next_line_utils_bonus.c

all: run sanitize clean

run:
	@$(CC) $(CFLAGS) test_gnl.c $(M_SRC) -o bin_m_default && ./bin_m_default
	@$(CC) $(CFLAGS) -D BUFFER_SIZE=0 test_gnl.c $(M_SRC) -o bin_m_0 && ./bin_m_0
	@$(CC) $(CFLAGS) -D BUFFER_SIZE=-42 test_gnl.c $(M_SRC) -o bin_m_neg && ./bin_m_neg
	@$(CC) $(CFLAGS) -D BUFFER_SIZE=1 test_gnl.c $(M_SRC) -o bin_m_1 && ./bin_m_1
	@$(CC) $(CFLAGS) -D BUFFER_SIZE=7 test_gnl.c $(M_SRC) -o bin_m_7 && ./bin_m_7
	@$(CC) $(CFLAGS) -D BUFFER_SIZE=9999 test_gnl.c $(M_SRC) -o bin_m_9999 && ./bin_m_9999
	@$(CC) $(CFLAGS) -D TEST_BONUS test_gnl.c $(B_SRC) -o bin_b_default && ./bin_b_default
	@$(CC) $(CFLAGS) -D TEST_BONUS -D BUFFER_SIZE=0 test_gnl.c $(B_SRC) -o bin_b_0 && ./bin_b_0
	@$(CC) $(CFLAGS) -D TEST_BONUS -D BUFFER_SIZE=1 test_gnl.c $(B_SRC) -o bin_b_1 && ./bin_b_1
	@$(CC) $(CFLAGS) -D TEST_BONUS -D BUFFER_SIZE=9999 test_gnl.c $(B_SRC) -o bin_b_9999 && ./bin_b_9999

sanitize:
	@$(CC) $(CFLAGS) $(SAN) -D BUFFER_SIZE=1 test_gnl.c $(M_SRC) -o bin_san_m1 && ./bin_san_m1
	@$(CC) $(CFLAGS) $(SAN) -D BUFFER_SIZE=42 test_gnl.c $(M_SRC) -o bin_san_m42 && ./bin_san_m42
	@$(CC) $(CFLAGS) $(SAN) -D BUFFER_SIZE=10000000 test_gnl.c $(M_SRC) -o bin_san_m10m && ./bin_san_m10m
	@$(CC) $(CFLAGS) $(SAN) -D TEST_BONUS -D BUFFER_SIZE=1 test_gnl.c $(B_SRC) -o bin_san_b1 && ./bin_san_b1
	@$(CC) $(CFLAGS) $(SAN) -D TEST_BONUS -D BUFFER_SIZE=42 test_gnl.c $(B_SRC) -o bin_san_b42 && ./bin_san_b42
	@$(CC) $(CFLAGS) $(SAN) -D TEST_BONUS -D BUFFER_SIZE=10000000 test_gnl.c $(B_SRC) -o bin_san_b10m && ./bin_san_b10m

clean fclean:
	@rm -f bin_* *.o

.PHONY: all run sanitize clean fclean
