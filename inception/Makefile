# ============================================================================ #
#  Inception — Makefile (42 School)                                           #
#  All docker compose operations go through here.                            #
# ============================================================================ #

COMPOSE      ?= docker compose
PROFILE      ?= ""
DOMAIN_NAME  ?= inception.42.fr

# Docker compose profile for bonus services
BONUS_FLAG   := --profile bonus

.PHONY: help up down stop build clean re ps logs test secrets-generate check

help: ## Show available targets
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-18s\033[0m %s\n", $$1, $$2}'

# ------------------------------------------------------------------- #
#  Core lifecycle                                                       #
# ------------------------------------------------------------------- #
up: ## Build and start the mandatory stack (NGINX + WordPress + MariaDB)
	@mkdir -p data/mariadb data/wordpress data/ftp
	$(COMPOSE) build
	$(COMPOSE) up -d

up-bonus: ## Build and start the full stack including bonus services
	@mkdir -p data/mariadb data/wordpress data/ftp
	$(COMPOSE) build
	$(COMPOSE) $(BONUS_FLAG) up -d

down: ## Stop and remove all containers
	$(COMPOSE) --profile bonus down

stop: ## Stop the containers (keep them)
	$(COMPOSE) --profile bonus stop

build: ## Build all images without starting
	$(COMPOSE) --profile bonus build

# ------------------------------------------------------------------- #
#  Maintenance                                                          #
# ------------------------------------------------------------------- #
re: clean build up ## Clean, rebuild and restart everything

clean: ## Stop containers, remove volumes and images
	$(COMPOSE) --profile bonus down -v --rmi local --remove-orphans

ps: ## Show running containers
	$(COMPOSE) --profile bonus ps

logs: ## Show logs (follow by setting FOLLOW=1)
ifeq ($(FOLLOW),1)
	$(COMPOSE) --profile bonus logs -f
else
	$(COMPOSE) --profile bonus logs
endif

# ------------------------------------------------------------------- #
#  Secrets & validation                                                  #
# ------------------------------------------------------------------- #
secrets-generate: ## (Re)generate all secret credentials
	@bash -c '\
		for s in db_password db_root_password wp_admin_password wp_user_password; do \
			pw=$$(LC_ALL=C tr -dc "A-Za-z0-9" < /dev/urandom | head -c 20); \
			printf "%s" "$$pw" > secrets/$$s.txt; \
			echo "  $$s -> secrets/$$s.txt"; \
		done'

check: ## Validate configuration (YAML syntax + structure)
	@python3 tests/check_config.py

test: ## Run the full test suite
	@python3 tests/run_tests.py
