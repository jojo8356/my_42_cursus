TIMEOUT		= 10
ifneq ($(shell command -v clang 2>/dev/null),)
CC_CMD		= clang
else
CC_CMD		= cc
endif
CFLAGS		= -g -Wall -Wextra -Werror -I../..
STRICT		=
UTILS 		= utils/malloc_mock.c utils/utils.c file_utils.c
TEST_FILES	= ../../get_next_line.c ../../get_next_line_utils.c
BONUS_FILES	= ../../get_next_line_bonus.c ../../get_next_line_utils_bonus.c
TIMEOUT_ARG	= -D TIMEOUT=$(TIMEOUT)
export TERM	?= xterm

ifdef EXEC_STRICT
STRICT		= -D STRICT_MEM
endif

CFLAGS		+= -Wno-deprecated-declarations

all: mandatory bonus clean

mandatory:
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=1 $(TIMEOUT_ARG) $(STRICT) tester.c $(UTILS) $(TEST_FILES) -ldl -o test1.out && ./test1.out < lines_around_10.txt
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=10 $(TIMEOUT_ARG) $(STRICT) tester.c $(UTILS) $(TEST_FILES) -ldl -o test2.out && ./test2.out < lines_around_10.txt
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=1000000 $(TIMEOUT_ARG) $(STRICT) tester.c $(UTILS) $(TEST_FILES) -ldl -o test3.out && ./test3.out < lines_around_10.txt

bonus:
	@tput setaf 3 && /bin/echo BONUS!
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=1 $(TIMEOUT_ARG) $(STRICT) tester.c $(UTILS) $(BONUS_FILES) -ldl -o test4.out && ./test4.out < lines_around_10.txt
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=10 $(TIMEOUT_ARG) $(STRICT) tester.c $(UTILS) $(BONUS_FILES) -ldl -o test5.out && ./test5.out < lines_around_10.txt
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=1000000 $(TIMEOUT_ARG) $(STRICT) tester.c $(UTILS) $(BONUS_FILES) -ldl -o test6.out && ./test6.out < lines_around_10.txt
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=1 $(TIMEOUT_ARG) $(STRICT) bonus.c $(UTILS) $(BONUS_FILES) -ldl -o test7.out && ./test7.out
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=10 $(TIMEOUT_ARG) $(STRICT) bonus.c $(UTILS) $(BONUS_FILES) -ldl -o test8.out && sh -c 'for fd in 3 4 5 6 7 8 9; do eval "exec $${fd}>&-"; done; exec ./test8.out'
	@$(CC_CMD) $(CFLAGS) -D BUFFER_SIZE=1000000 $(TIMEOUT_ARG) $(STRICT) bonus.c $(UTILS) $(BONUS_FILES) -ldl -o test9.out && ./test9.out

clean fclean:
	@rm -f test*.out limits.txt errors.log error_color.log

.PHONY: all mandatory bonus clean fclean
