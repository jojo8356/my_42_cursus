#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BOLD='\033[1m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
RESET='\033[0m'

printf "${BOLD}${CYAN}============================================================================${RESET}\n"
printf "${BOLD}${CYAN}   SUITE DE TESTS EXHAUSTIVE — ECOLE 42 LIBFT 2026 (SUJET VERSION 19.3)     ${RESET}\n"
printf "${BOLD}${CYAN}============================================================================${RESET}\n\n"

# Phase 1: Setup, Norminette, Prototypes, Makefile, Relink, nm Symbol Audit, README.md
./test_setup.sh

# Phase 2: Unit & Edge-Case Tests on ALL 43 Functions (Standard Build + malloc_usable_size)
make -s fclean
make -s unit

# Phase 3: Full Recompilation & Execution under AddressSanitizer + UBSan + LeakSanitizer
printf "${BOLD}${CYAN}=== PHASE 3 : VERIFICATION MEMOIRE & UB (ASAN + UBSAN + LEAKSANITIZER) ===${RESET}\n"
make -s asan

# Phase 4: Deterministic malloc() Fault Injection across ALL 10 Allocating Functions
make -s alloc_fail

# Leave libft/ in clean built state and verify zero stray files in libft/
make -s -C ../libft re >/dev/null

printf "${BOLD}${GREEN}============================================================================${RESET}\n"
printf "${BOLD}${GREEN}   SUCCES TOTAL (100%%) : LES 4 PHASES ET LES 43/43 FONCTIONS SONT VALIDEES  ${RESET}\n"
printf "${BOLD}${GREEN}============================================================================${RESET}\n"
