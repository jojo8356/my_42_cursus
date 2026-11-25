/*
 * searchable_tree_bag.hpp — 42 Exam Rank 05 (level 0), polyset
 * A tree_bag that can search for values (implements searchable_bag).
 */
#ifndef SEARCHABLE_TREE_BAG_HPP
#define SEARCHABLE_TREE_BAG_HPP

#include "given-code/tree_bag.hpp"
#include "given-code/searchable_bag.hpp"

class searchable_tree_bag : public tree_bag, public searchable_bag
{
public:
    searchable_tree_bag();
    searchable_tree_bag(const searchable_tree_bag &other);
    searchable_tree_bag &operator=(const searchable_tree_bag &other);
    ~searchable_tree_bag();

    void insert(int value);
    void insert(int *arr, int n);

    bool has(int value) const;
};

#endif /* SEARCHABLE_TREE_BAG_HPP */
