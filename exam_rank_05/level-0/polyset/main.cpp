/*
 * main.cpp — 42 Exam Rank 05 (level 0), polyset
 * Demonstration of the searchable bags and set.
 */
#include "searchable_array_bag.hpp"
#include "searchable_tree_bag.hpp"
#include "set.hpp"

#include <cstdlib>

int main(int ac, char **av)
{
    searchable_array_bag abag;
    for (int i = 1; i < ac; i++)
        abag.insert(std::atoi(av[i]));

    set s(abag);
    s.insert(1);
    s.insert(2);
    s.insert(3);
    s.insert(4);

    (void)s;
    return 0;
}
