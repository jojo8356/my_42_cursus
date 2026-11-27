.PHONY: install run test clean

install:
	pip install -r requirements.txt

run:
	python3 pac-man.py src/config.json

test:
	python3 -m pytest tests/ -v

clean:
	find . -type d -name __pycache__ -exec rm -rf {} +
	find . -type d -name .pytest_cache -exec rm -rf {} +
	rm -f highscores.json
