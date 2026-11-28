.PHONY: install install-dev run test coverage lint typecheck check clean

install:
	pip install -r requirements.txt

install-dev:
	pip install -r requirements-dev.txt

run:
	python3 pac-man.py src/config.json

test:
	python3 -m pytest tests/ -v

coverage:
	python3 -m pytest tests/ --cov=src --cov=pacman_cli --cov-report=term-missing

lint:
	python3 -m flake8 .

typecheck:
	python3 -m mypy

check: test lint typecheck

clean:
	find . -type d -name __pycache__ -exec rm -rf {} +
	find . -type d -name .pytest_cache -exec rm -rf {} +
	rm -f highscores.json
