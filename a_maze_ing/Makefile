PYTHON ?= python3

.PHONY: all run debug clean lint install

all: run

lint:
	flake8 .
	mypy . --warn-return-any --warn-unused-ignores --ignore-missing-imports --disallow-untyped-defs --check-untyped-defs

run:
	$(PYTHON) a_maze_ing.py config.txt

debug:
	$(PYTHON) -m pdb a_maze_ing.py config.txt

clean:
	find . -name __pycache__ -type d -exec rm -rf {} +
	rm -rf .mypy_cache

install:
	$(PYTHON) -m pip install -r requirements.txt
