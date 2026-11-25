/*
 * main.cpp — 42 Exam Rank 05 (new 2025), level 0 — vect2
 * Demonstration of the vect2 class.
 */
#include "vect2.hpp"

int main()
{
    vect2 v2(1, 2);
    vect2 v3(1, 2);

    v2 += v2 += v3;

    std::cout << v2 << std::endl;
    return 0;
}
