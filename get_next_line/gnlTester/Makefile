TIMEOUT_US				= 1000000
.DEFAULT_GOAL			= a
UTILS					= utils/sigsegv.cpp utils/color.cpp utils/check.cpp utils/gnl.cpp utils/leaks.cpp
TESTS_PATH				= tests/
SHELL					= bash
export TERM				?= xterm

MANDATORY_HEADER		= ../get_next_line.h
MANDATORY_FILES			= ../get_next_line.c ../get_next_line_utils.c
MANDATORY_OBJS			= $(MANDATORY_FILES:../%.c=%.o)
M1_OBJS					= get_next_line.bs1.o get_next_line_utils.bs1.o
M42_OBJS				= get_next_line.bs42.o get_next_line_utils.bs42.o
M10M_OBJS				= get_next_line.bs10M.o get_next_line_utils.bs10M.o

BONUS_HEADER			= ../get_next_line_bonus.h
BONUS_FILES				= ../get_next_line_bonus.c ../get_next_line_utils_bonus.c
BONUS_OBJS				= $(BONUS_FILES:../%.c=%.o)
B1_OBJS					= get_next_line_bonus.bs1.o get_next_line_utils_bonus.bs1.o
B42_OBJS				= get_next_line_bonus.bs42.o get_next_line_utils_bonus.bs42.o
B10M_OBJS				= get_next_line_bonus.bs10M.o get_next_line_utils_bonus.bs10M.o

MANDATORY				= mandatory
1MANDATORY				= $(addprefix 1, $(MANDATORY))
42MANDATORY				= $(addprefix 42, $(MANDATORY))
10MMANDATORY			= $(addprefix 10M, $(MANDATORY))

BONUS					= bonus
1MBONUS					= $(addprefix m1, $(BONUS))
42MBONUS				= $(addprefix m42, $(BONUS))
10MMBONUS				= $(addprefix m10M, $(BONUS))

1BONUS					= $(addprefix 1, $(BONUS))
42BONUS					= $(addprefix 42, $(BONUS))
10MBONUS				= $(addprefix 10M, $(BONUS))

CC						= cc
ifneq ($(shell command -v clang++ 2>/dev/null),)
CXX_CMD					= clang++ -Wno-everything
else
CXX_CMD					= g++ -w
endif

CFLAGS					= -g3 -Wall -Wextra -Werror
CPPFLAGS				= -g3 -ldl -std=c++11 -I utils/ -I..

UNAME = $(shell uname -s)
ifeq ($(UNAME), Linux)
ifneq ($(shell command -v valgrind 2>/dev/null),)
    VALGRIND = valgrind -q --leak-check=full
endif
endif

$(1MANDATORY): 1%:
	@$(CC) -D BUFFER_SIZE=1 $(CFLAGS) -c ../get_next_line.c -o get_next_line.bs1.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=1 $(CFLAGS) -c ../get_next_line_utils.c -o get_next_line_utils.bs1.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=1 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$*.cpp $(M1_OBJS) -o gnlTest_bs1 && $(VALGRIND) ./gnlTest_bs1 < files/alternate_line_nl_with_nl && rm -f gnlTest_bs1
$(42MANDATORY): 42%:
	@$(CC) -D BUFFER_SIZE=42 $(CFLAGS) -c ../get_next_line.c -o get_next_line.bs42.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=42 $(CFLAGS) -c ../get_next_line_utils.c -o get_next_line_utils.bs42.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=42 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$*.cpp $(M42_OBJS) -o gnlTest_bs42 && $(VALGRIND) ./gnlTest_bs42 < files/alternate_line_nl_with_nl && rm -f gnlTest_bs42
$(10MMANDATORY): 10M%:
	@$(CC) -D BUFFER_SIZE=10000000 $(CFLAGS) -c ../get_next_line.c -o get_next_line.bs10M.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=10000000 $(CFLAGS) -c ../get_next_line_utils.c -o get_next_line_utils.bs10M.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=10000000 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$*.cpp $(M10M_OBJS) -o gnlTest_bs10M && $(VALGRIND) ./gnlTest_bs10M < files/alternate_line_nl_with_nl && rm -f gnlTest_bs10M

$(1BONUS): 1%:
	@$(CC) -D BUFFER_SIZE=1 $(CFLAGS) -c ../get_next_line_bonus.c -o get_next_line_bonus.bs1.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=1 $(CFLAGS) -c ../get_next_line_utils_bonus.c -o get_next_line_utils_bonus.bs1.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=1 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$*.cpp $(B1_OBJS) -o gnlTest_bbs1 && $(VALGRIND) ./gnlTest_bbs1 < files/alternate_line_nl_with_nl && rm -f gnlTest_bbs1
$(42BONUS): 42%:
	@$(CC) -D BUFFER_SIZE=42 $(CFLAGS) -c ../get_next_line_bonus.c -o get_next_line_bonus.bs42.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=42 $(CFLAGS) -c ../get_next_line_utils_bonus.c -o get_next_line_utils_bonus.bs42.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=42 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$*.cpp $(B42_OBJS) -o gnlTest_bbs42 && $(VALGRIND) ./gnlTest_bbs42 < files/alternate_line_nl_with_nl && rm -f gnlTest_bbs42
$(10MBONUS): 10M%:
	@$(CC) -D BUFFER_SIZE=10000000 $(CFLAGS) -c ../get_next_line_bonus.c -o get_next_line_bonus.bs10M.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=10000000 $(CFLAGS) -c ../get_next_line_utils_bonus.c -o get_next_line_utils_bonus.bs10M.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=10000000 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$*.cpp $(B10M_OBJS) -o gnlTest_bbs10M && $(VALGRIND) ./gnlTest_bbs10M < files/alternate_line_nl_with_nl && rm -f gnlTest_bbs10M

$(1MBONUS): m1%:
	@$(CC) -D BUFFER_SIZE=1 $(CFLAGS) -c ../get_next_line_bonus.c -o get_next_line_bonus.bs1.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=1 $(CFLAGS) -c ../get_next_line_utils_bonus.c -o get_next_line_utils_bonus.bs1.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=1 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$(MANDATORY).cpp $(B1_OBJS) -o gnlTest_mbs1 && $(VALGRIND) ./gnlTest_mbs1 < files/alternate_line_nl_with_nl && rm -f gnlTest_mbs1
$(42MBONUS): m42%:
	@$(CC) -D BUFFER_SIZE=42 $(CFLAGS) -c ../get_next_line_bonus.c -o get_next_line_bonus.bs42.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=42 $(CFLAGS) -c ../get_next_line_utils_bonus.c -o get_next_line_utils_bonus.bs42.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=42 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$(MANDATORY).cpp $(B42_OBJS) -o gnlTest_mbs42 && $(VALGRIND) ./gnlTest_mbs42 < files/alternate_line_nl_with_nl && rm -f gnlTest_mbs42
$(10MMBONUS): m10M%:
	@$(CC) -D BUFFER_SIZE=10000000 $(CFLAGS) -c ../get_next_line_bonus.c -o get_next_line_bonus.bs10M.o -gdwarf-4
	@$(CC) -D BUFFER_SIZE=10000000 $(CFLAGS) -c ../get_next_line_utils_bonus.c -o get_next_line_utils_bonus.bs10M.o -gdwarf-4
	@$(CXX_CMD) -D BUFFER_SIZE=10000000 -gdwarf-4 -D TIMEOUT_US=$(TIMEOUT_US) $(CPPFLAGS) $(UTILS) $(TESTS_PATH)$(MANDATORY).cpp $(B10M_OBJS) -o gnlTest_mbs10M && $(VALGRIND) ./gnlTest_mbs10M < files/alternate_line_nl_with_nl && rm -f gnlTest_mbs10M

mandatory_start: update
	@for file in $(MANDATORY_FILES); do \
		grep -qE '#[[:space:]]*include[[:space:]]*"get_next_line\.h"' $$file \
		|| { tput setaf 1; echo "KO: $$file must include \"get_next_line.h\""; tput sgr0; exit 1; }; \
	done
	@tput setaf 4 && echo [Mandatory]

bonus_start: update
	@for file in $(BONUS_FILES); do \
		grep -qE '#[[:space:]]*include[[:space:]]*"get_next_line_bonus\.h"' $$file \
		|| { tput setaf 1; echo "KO: $$file must include \"get_next_line_bonus.h\""; tput sgr0; exit 1; }; \
	done
	@tput setaf 3 && printf "[Static = " && cat ../*bonus.c | grep -E 'static.*;' | wc -l | tr -d '\n' | xargs /bin/echo -n && printf "]\n"
	@tput setaf 5 && /bin/echo [Bonus]

update:
	@true

m: mandatory_start $(1MANDATORY) $(42MANDATORY) $(10MMANDATORY) cleanMandatory
b: bonus_start $(1MBONUS) $(42MBONUS) $(10MMBONUS) $(1BONUS) $(42BONUS) $(10MBONUS) cleanBonus
a: m b

fclean clean cleanMandatory cleanBonus:
	@rm -rf $(MANDATORY_OBJS) $(BONUS_OBJS) *.o gnlTest*

.PHONY: mandatory_start m bonus_start b a fclean clean cleanMandatory cleanBonus update
