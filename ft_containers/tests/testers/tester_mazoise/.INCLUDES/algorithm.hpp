#ifndef ALGORITHM_HPP
# define ALGORITHM_HPP

//////////////////////////////////////////////////////////////////////////
//  algorithm.hpp — lexicographical_compare, equal, distance, iter_swap
//  (C++98, namespace ft)
//////////////////////////////////////////////////////////////////////////

# include <iterator>
# include "iterator.hpp"
# include "utility.hpp"

namespace ft
{

template <class InputIterator1, class InputIterator2>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
                             InputIterator2 first2, InputIterator2 last2)
{
    while (first1 != last1 && first2 != last2)
    {
        if (*first1 < *first2)
            return true;
        if (*first2 < *first1)
            return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 != last2);
}

template <class InputIterator1, class InputIterator2>
bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
{
    while (first1 != last1)
    {
        if (*first1 != *first2)
            return false;
        ++first1;
        ++first2;
    }
    return true;
}

template <class InputIterator1, class InputIterator2>
bool equal(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2)
{
    while (first1 != last1 && first2 != last2)
    {
        if (*first1 != *first2)
            return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

// Optimized distance for random-access iterators
template <class RandomAccessIterator>
typename iterator_traits<RandomAccessIterator>::difference_type
distance(RandomAccessIterator first, RandomAccessIterator last,
         ft::random_access_iterator_tag)
{
    return last - first;
}

template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last, ft::input_iterator_tag)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last)
    {
        ++first;
        ++n;
    }
    return n;
}

template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last, std::input_iterator_tag)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last)
    {
        ++first;
        ++n;
    }
    return n;
}

template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
    return distance(first, last,
                    typename iterator_traits<InputIterator>::iterator_category());
}

template <class Iterator1, class Iterator2>
void iter_swap(Iterator1 a, Iterator2 b)
{
    ft::swap(*a, *b);
}

} // namespace ft

#endif // ALGORITHM_HPP
