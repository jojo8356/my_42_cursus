CONTAINERS = vector list map stack queue deque multimap set multiset

FLAGS = -Wall -Wextra -Werror -std=c++98 -Wno-error=catch-value -Wno-error=exceptions
INCLUDES = -I.. -I../../../includes

all: $(CONTAINERS)

# Chaque test est valide si ft et std ont le même résultat de compilation
# (certains tests sont intentionnellement invalides et échouent des deux côtés).
$(CONTAINERS):
	@fail=0; \
	for f in srcs/$@/*.cpp; do \
		g++ $(FLAGS) $(INCLUDES) -DTESTED_NAMESPACE=ft -c $$f -o /dev/null 2>/dev/null; \
		ft=$$?; \
		g++ $(FLAGS) $(INCLUDES) -DTESTED_NAMESPACE=std -c $$f -o /dev/null 2>/dev/null; \
		std=$$?; \
		if [ $$ft -ne $$std ]; then \
			echo "DIVERGENCE de compile : $$f (ft=$$ft std=$$std)"; \
			fail=1; \
		fi; \
	done; \
	exit $$fail

clean:
	@true

fclean: clean

re: fclean all

.PHONY: all clean fclean re
