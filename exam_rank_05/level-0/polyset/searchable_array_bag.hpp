/*
 * searchable_array_bag.hpp — 42 Exam Rank 05 (level 0), polyset
 * An array_bag that can search for values (implements searchable_bag).
 */
#ifndef SEARCHABLE_ARRAY_BAG_HPP
#define SEARCHABLE_ARRAY_BAG_HPP

#include "given-code/array_bag.hpp"
#include "given-code/searchable_bag.hpp"

class searchable_array_bag : public array_bag, public searchable_bag
{
public:
    searchable_array_bag();
    searchable_array_bag(const searchable_array_bag &other);
    searchable_array_bag &operator=(const searchable_array_bag &other);
    ~searchable_array_bag();

    void insert(int value);
    void insert(int *arr, int n);

    bool has(int value) const;
};

#endif /* SEARCHABLE_ARRAY_BAG_HPP */
