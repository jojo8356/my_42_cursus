# Top-level Makefile for Exam Rank 03
# Builds all 8 programs (level 1 mandatory + level 2 bonus).

PROGS = filter ft_scanf get_next_line n_queens permutations powerset rip tsp

all: $(PROGS)

$(PROGS):
	@$(MAKE) --no-print-directory -C $@ all

clean:
	@for p in $(PROGS); do $(MAKE) --no-print-directory -C $$p clean; done

fclean:
	@for p in $(PROGS); do $(MAKE) --no-print-directory -C $$p fclean; done

re: fclean
	@$(MAKE) --no-print-directory all

.PHONY: all clean fclean re $(PROGS)
