NAME        = fly-in
PYTHON      = python3
LINT_FLAGS  = --warn-return-any --warn-unused-ignores --ignore-missing-imports \
              --disallow-untyped-defs --check-untyped-defs

.PHONY: install run debug clean lint lint-strict test

install: ## Install the project dependencies (dev tools included)
	$(PYTHON) -m pip install -r requirements.txt

run: ## Run the main script (override the map with MAP=...)
	$(PYTHON) main.py $(if $(MAP),$(MAP),)

debug: ## Run the main script under the Python debugger
	$(PYTHON) -m pdb main.py $(if $(MAP),$(MAP),)

clean: ## Remove bytecode and tool caches
	rm -rf __pycache__ src/__pycache__ tests/__pycache__ .mypy_cache .pytest_cache

lint: ## flake8 + mypy (subject-required flags)
	$(PYTHON) -m flake8 .
	$(PYTHON) -m mypy . $(LINT_FLAGS)

lint-strict: ## flake8 + mypy --strict
	$(PYTHON) -m flake8 .
	$(PYTHON) -m mypy . --strict

test: ## Run the full pytest suite
	$(PYTHON) -m pytest tests/ -q
