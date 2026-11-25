/*
 * set.cpp — 42 Exam Rank 05 (level 0), polyset
 */
#include "set.hpp"

set::set(searchable_bag &bag) : m_bag(&bag) {}

set::set(const set &other) : m_bag(other.m_bag) {}

set &set::operator=(const set &other)
{
    if (this != &other)
        m_bag = other.m_bag;
    return *this;
}

set::~set() {}

void set::insert(int value)
{
    if (!m_bag->has(value))
        m_bag->insert(value);
}

void set::insert(int *arr, int n)
{
    for (int i = 0; i < n; i++)
        insert(arr[i]);
}

bool set::has(int value) const
{
    return m_bag->has(value);
}

searchable_bag &set::get_bag() const
{
    return *m_bag;
}
