CC			= cc
CFG			= -Wall -Wextra -Werror -I. -pthread
SRC			= sq_main.c sq_get_next_line.c sq_get_next_line_utils.c

all: setup exec test fco

exec: $(SRC)
	@$(CC) -o t0.esq $^ $(CFG) -D BUFFER_SIZE=0
	@$(CC) -o t1.esq $^ $(CFG) -D BUFFER_SIZE=1
	@$(CC) -o t2.esq $^ $(CFG) -D BUFFER_SIZE=2
	@$(CC) -o t42.esq $^ $(CFG) -D BUFFER_SIZE=42
	@$(CC) -o t42000.esq $^ $(CFG) -D BUFFER_SIZE=42000

setup: clean
	@cp ../../get_next_line.h ./get_next_line.h
	@cp ../../get_next_line.c ./sq_get_next_line.c
	@cp ../../get_next_line_utils.c ./sq_get_next_line_utils.c

test:
	@sh ./sloquet.sh

clean:
	@rm -f get_next_line.h sq_get_next_line.c sq_get_next_line_utils.c *.esq log_norminette

fco fclean: clean
	@rm -rf out_*

.PHONY: all exec setup test clean fco fclean
