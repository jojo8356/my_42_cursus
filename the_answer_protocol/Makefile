PYTHON ?= python3
HOST ?= 127.0.0.1
PORT ?= 4242
GUI_HOST ?= 0.0.0.0
GUI_PORT ?= 8080
WORLD ?= data/world.json

.PHONY: all install test coverage lint check-world run-server run-client run-client-gui demo clean help

all: help

install:
	$(PYTHON) -m pip install -r requirements.txt

test:
	PYTHONPATH=src $(PYTHON) -m pytest tests -q

coverage:
	$(PYTHON) -m coverage erase
	PYTHONPATH=src $(PYTHON) -m coverage run --branch --source=src -m pytest tests -q
	$(PYTHON) -m coverage report --fail-under=100

lint:
	$(PYTHON) -m compileall -q src tap.py tests

check-world:
	$(PYTHON) tap.py check-world --world $(WORLD)

run-server:
	$(PYTHON) tap.py server --host $(HOST) --port $(PORT) --world $(WORLD)

run-client:
	$(PYTHON) tap.py client --host $(HOST) --port $(PORT)

run-client-gui:
	$(PYTHON) tap.py gui --host $(GUI_HOST) --port $(GUI_PORT) --tap-host $(HOST) --tap-port $(PORT)

demo:
	$(PYTHON) tap.py demo --world $(WORLD)

clean:
	find . -type d -name __pycache__ -prune -exec rm -rf {} +
	rm -rf .pytest_cache .coverage htmlcov

help:
	@printf '%s\n' \
	  'The Answer Protocol (stdlib-only server/client)' \
	  '  make install          install optional test runner (pytest)' \
	  '  make test             run the full test suite' \
	  '  make coverage         run tests and require 100% line/branch coverage' \
	  '  make lint             compile-check project Python files' \
	  '  make check-world      validate data/world.json' \
	  '  make run-server       start TCP server on $(HOST):$(PORT)' \
	  '  make run-client       start terminal client' \
	  '  make run-client-gui   start GUI at $(GUI_HOST):$(GUI_PORT)' \
	  '  make demo             run a scripted two-player TCP session' \
	  '  make clean            remove generated Python/test caches'
