# exam_rank_05 — top-level Makefile
# Builds all 5 projects: vect2, bigint, polyset (C++), Life, BSQ (C)

CXX       = g++
CC        = gcc
CXXFLAGS  = -std=c++98 -Wall -Wextra -Werror
CFLAGS    = -Wall -Wextra -Werror

PROJECTS  = level-0/vect2 level-0/bigint level-0/polyset level-1/Life level-1/BSQ

all: $(PROJECTS)

level-0/vect2:
	$(MAKE) -C level-0/vect2 all

level-0/bigint:
	$(MAKE) -C level-0/bigint all

level-0/polyset:
	$(MAKE) -C level-0/polyset all

level-1/Life:
	$(MAKE) -C level-1/Life all

level-1/BSQ:
	$(MAKE) -C level-1/BSQ all

clean:
	@for p in $(PROJECTS); do $(MAKE) -C $$p clean; done

.PHONY: all clean $(PROJECTS)
