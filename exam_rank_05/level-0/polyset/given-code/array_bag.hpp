/*
 * array_bag.hpp — 42 Exam Rank 05 (level 0), polyset given-code
 * Base bag storing elements in a dynamic array.
 */
#ifndef ARRAY_BAG_HPP
#define ARRAY_BAG_HPP

class array_bag
{
protected:
    int *m_data;
    int m_size;
    int m_capacity;

    void grow();

public:
    array_bag();
    array_bag(const array_bag &other);
    array_bag &operator=(const array_bag &other);
    virtual ~array_bag();

    void insert(int value);
    void insert(int *arr, int n);

    int count() const;
    bool empty() const;
    void clear();
};

#endif /* ARRAY_BAG_HPP */
