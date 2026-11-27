PYTHON ?= python3
TASK   ?= data/mbpp_task.example.json
OUTPUT ?= solution.json

.PHONY: help install test lint run_mbpp run_swebench evaluate tools sandbox clean

help:
	@echo "make install      install the test dependencies"
	@echo "make test         run the whole test suite"
	@echo "make run_mbpp     run the scripted MBPP demo"
	@echo "make run_swebench run the scripted SWE-bench demo"
	@echo "make evaluate     evaluate the example MBPP benchmark"
	@echo "make tools        list the MBPP tools"
	@echo "make sandbox      start the sandbox REPL"

install:
	$(PYTHON) -m pip install -r requirements.txt

test:
	$(PYTHON) -m pytest tests/ -v

lint:
	$(PYTHON) -m compileall -q src agent_smith.py

run_mbpp:
	$(PYTHON) agent_smith.py run --benchmark mbpp \
		--task-file $(TASK) --output $(OUTPUT) --scripted data/scripted_mbpp.json

run_swebench:
	$(PYTHON) agent_smith.py --repo . run --benchmark swebench \
		--task-file data/swe_task.example.json --output $(OUTPUT) \
		--scripted data/scripted_swebench.json

evaluate:
	$(PYTHON) agent_smith.py evaluate --benchmark mbpp \
		--tasks-file data/mbpp_bench.example.json \
		--scripted data/scripted_mbpp.json --output report.json

tools:
	$(PYTHON) agent_smith.py tools --mcp mbpp --manual

sandbox:
	$(PYTHON) agent_smith.py sandbox --mcp mbpp

clean:
	rm -rf __pycache__ src/__pycache__ tests/__pycache__ .pytest_cache
	rm -f solution.json report.json
