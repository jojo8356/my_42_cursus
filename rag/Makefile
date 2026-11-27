.PHONY: install index index_hybrid search answer evaluate api interface test clean

install:
	pip install -r requirements.txt

index:
	python3 rag.py index

index_hybrid:
	python3 rag.py index --hybrid

search:
	python3 rag.py search "$(Q)"

answer:
	python3 rag.py answer "$(Q)"

evaluate:
	python3 rag.py evaluate --results $(RESULTS) --dataset $(DATASET)

api:
	python3 rag.py api

interface:
	python3 rag.py interface

test:
	python3 -m pytest tests/ -v

clean:
	rm -rf data/processed data/*.json
	find . -type d -name __pycache__ -exec rm -rf {} +
	find . -type d -name .pytest_cache -exec rm -rf {} +
