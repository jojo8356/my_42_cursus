/*
 * array_bag.cpp — 42 Exam Rank 05 (level 0), polyset given-code
 */
#include "array_bag.hpp"

#include <cstring>

void array_bag::grow()
{
    int new_cap = (m_capacity == 0) ? 4 : m_capacity * 2;
    int *new_data = new int[new_cap];
    for (int i = 0; i < m_size; i++)
        new_data[i] = m_data[i];
    delete[] m_data;
    m_data = new_data;
    m_capacity = new_cap;
}

array_bag::array_bag() : m_data(NULL), m_size(0), m_capacity(0) {}

array_bag::array_bag(const array_bag &other)
    : m_data(NULL), m_size(other.m_size), m_capacity(0)
{
    if (other.m_size > 0)
    {
        m_capacity = other.m_size;
        m_data = new int[m_capacity];
        for (int i = 0; i < m_size; i++)
            m_data[i] = other.m_data[i];
    }
}

array_bag &array_bag::operator=(const array_bag &other)
{
    if (this != &other)
    {
        delete[] m_data;
        m_data = NULL;
        m_size = other.m_size;
        m_capacity = 0;
        if (m_size > 0)
        {
            m_capacity = m_size;
            m_data = new int[m_capacity];
            for (int i = 0; i < m_size; i++)
                m_data[i] = other.m_data[i];
        }
    }
    return *this;
}

array_bag::~array_bag()
{
    delete[] m_data;
}

void array_bag::insert(int value)
{
    if (m_size >= m_capacity)
        grow();
    m_data[m_size++] = value;
}

void array_bag::insert(int *arr, int n)
{
    for (int i = 0; i < n; i++)
        insert(arr[i]);
}

int array_bag::count() const
{
    return m_size;
}

bool array_bag::empty() const
{
    return m_size == 0;
}

void array_bag::clear()
{
    m_size = 0;
}
