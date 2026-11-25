/*
 * set.hpp — 42 Exam Rank 05 (level 0), polyset
 * A set that wraps a searchable_bag and prevents duplicate insertions.
 */
#ifndef SET_HPP
#define SET_HPP

#include "given-code/searchable_bag.hpp"

class set
{
private:
    searchable_bag *m_bag;

public:
    set(searchable_bag &bag);
    set(const set &other);
    set &operator=(const set &other);
    ~set();

    void insert(int value);
    void insert(int *arr, int n);

    bool has(int value) const;
    searchable_bag &get_bag() const;
};

#endif /* SET_HPP */
