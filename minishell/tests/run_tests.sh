#!/usr/bin/env bash
# ****************************************************************************
#                                                                       #
#                                                        :::      ::::::::   #
#   run_tests.sh                                        :+:      :+:    :+:   #
#                                                    +:+ +:+         +:+      #
#   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        #
#                                                +#+#+#+#+#+   +#+           #
#   Created: 2026/11/12 15:45:00 by jojo8356          #+#    #+#             #
#   Updated: 2026/11/12 15:45:00 by jojo8356         ###   ########.fr      #
#                                                                       #
# ****************************************************************************

cd "$(dirname "$0")/.."

echo "========================================="
echo "  Minishell Test Suite"
echo "========================================="
echo ""

# Build the project
echo "Building minishell..."
make fclean > /dev/null 2>&1
make > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "BUILD FAILED"
    exit 1
fi
echo "Build OK"
echo ""

# Compile and run tests
FAILED=0

compile_and_run() {
    local test_name=$1
    local test_file=$2
    echo "Running $test_name..."
    cc -I includes -I libft -I tests -o "tests/$test_name" "$test_file" 2> /tmp/compile_err.txt
    if [ $? -ne 0 ]; then
        echo "COMPILE FAILED: $test_name"
        cat /tmp/compile_err.txt
        FAILED=1
        return
    fi
    "tests/$test_name"
    if [ $? -ne 0 ]; then
        FAILED=1
    fi
    echo ""
}

compile_and_run "test_basic" "tests/test_basic.c"
compile_and_run "test_variables" "tests/test_variables.c"
compile_and_run "test_quotes" "tests/test_quotes.c"
compile_and_run "test_pipes" "tests/test_pipes.c"
compile_and_run "test_redirects" "tests/test_redirects.c"
compile_and_run "test_logical" "tests/test_logical.c"
compile_and_run "test_wildcards" "tests/test_wildcards.c"
compile_and_run "test_heredoc" "tests/test_heredoc.c"
compile_and_run "test_builtins" "tests/test_builtins.c"

echo "========================================="
if [ $FAILED -eq 0 ]; then
    echo "  ALL TEST SUITES PASSED"
    echo "========================================="
else
    echo "  SOME TESTS FAILED"
    echo "========================================="
fi
exit $FAILED
