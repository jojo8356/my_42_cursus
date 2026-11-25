/*
 * main.cpp — 42 Exam Rank 05 (new 2025), level 0 — bigint
 * Demonstration of the bigint class.
 */
#include "bigint.hpp"

int main()
{
    bigint result = (bigint(23) << 9) + 42;

    std::cout << result << std::endl;
    return 0;
}
