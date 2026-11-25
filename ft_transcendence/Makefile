.PHONY: install migrate run test clean

install:
	pip install -r requirements.txt

migrate:
	python manage.py migrate

run:
	python manage.py runserver 0.0.0.0:8000

test:
	python tests/run_tests.py

clean:
	find . -type d -name __pycache__ -exec rm -rf {} +
	rm -f db.sqlite3
