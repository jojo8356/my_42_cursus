CONTAINERS = VECTOR DEQUE LIST MAP SET STACK QUEUE MULTIMAP MULTISET

FLAGS = -Werror -Wextra -std=c++98 -Wno-error=catch-value -Wno-error=exceptions
INCLUDES = -I . -I ../.INCLUDES -I ../.INCLUDES/EMPTY -I .. -I ../../../includes -I ../.INCLUDES/EMPTY/JUST_IN_CASE

all: $(CONTAINERS)

$(CONTAINERS):
	cd $@ && for f in mains/*_main.cpp; do \
		g++ $(FLAGS) $(INCLUDES) -c $$f -o /dev/null || exit 1; \
	done

clean:
	@true

fclean: clean

re: fclean all

.PHONY: all clean fclean re
