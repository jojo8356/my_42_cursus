# exam_rank_06 — top-level Makefile
# Builds both projects: mini_serv (C) and mini_db (C++)

.PHONY: all build test clean mini_serv mini_db

all: build

build: mini_serv mini_db

mini_serv:
	$(MAKE) -C mini_serv all

mini_db:
	$(MAKE) -C mini_db all

test:
	python3 tests/run_tests.py

clean:
	$(MAKE) -C mini_serv clean
	$(MAKE) -C mini_db clean
