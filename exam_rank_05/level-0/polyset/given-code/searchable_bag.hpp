/*
 * searchable_bag.hpp — 42 Exam Rank 05 (level 0), polyset given-code
 * Abstract interface for a bag that can search for a value and insert.
 */
#ifndef SEARCHABLE_BAG_HPP
#define SEARCHABLE_BAG_HPP

class searchable_bag
{
public:
    virtual bool has(int value) const = 0;
    virtual void insert(int value) = 0;
    virtual void insert(int *arr, int n) = 0;
    virtual ~searchable_bag() {}
};

#endif /* SEARCHABLE_BAG_HPP */
