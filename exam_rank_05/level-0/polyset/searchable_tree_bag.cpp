/*
 * searchable_tree_bag.cpp — 42 Exam Rank 05 (level 0), polyset
 */
#include "searchable_tree_bag.hpp"

searchable_tree_bag::searchable_tree_bag() {}

searchable_tree_bag::searchable_tree_bag(const searchable_tree_bag &other)
    : tree_bag(other), searchable_bag() {}

searchable_tree_bag &searchable_tree_bag::operator=(const searchable_tree_bag &other)
{
    if (this != &other)
        tree_bag::operator=(other);
    return *this;
}

searchable_tree_bag::~searchable_tree_bag() {}

void searchable_tree_bag::insert(int value)
{
    tree_bag::insert(value);
}

void searchable_tree_bag::insert(int *arr, int n)
{
    tree_bag::insert(arr, n);
}

bool searchable_tree_bag::has(int value) const
{
    return has_node(m_root, value);
}
