/*
 * searchable_array_bag.cpp — 42 Exam Rank 05 (level 0), polyset
 */
#include "searchable_array_bag.hpp"

searchable_array_bag::searchable_array_bag() {}

searchable_array_bag::searchable_array_bag(const searchable_array_bag &other)
    : array_bag(other), searchable_bag() {}

searchable_array_bag &searchable_array_bag::operator=(const searchable_array_bag &other)
{
    if (this != &other)
        array_bag::operator=(other);
    return *this;
}

searchable_array_bag::~searchable_array_bag() {}

void searchable_array_bag::insert(int value)
{
    array_bag::insert(value);
}

void searchable_array_bag::insert(int *arr, int n)
{
    array_bag::insert(arr, n);
}

bool searchable_array_bag::has(int value) const
{
    for (int i = 0; i < count(); i++)
    {
        if (m_data[i] == value)
            return true;
    }
    return false;
}
