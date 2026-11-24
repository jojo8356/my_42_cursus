#ifndef ITERATOR_HPP
# define ITERATOR_HPP

//////////////////////////////////////////////////////////////////////////
//  iterator.hpp — iterator_traits, reverse_iterator (C++98, namespace ft)
//////////////////////////////////////////////////////////////////////////

# include "type_traits.hpp"

namespace ft
{

// --------------------------- iterator_traits -------------------------
template <typename Iterator>
struct iterator_traits
{
    typedef typename Iterator::value_type        value_type;
    typedef typename Iterator::difference_type   difference_type;
    typedef typename Iterator::pointer           pointer;
    typedef typename Iterator::reference         reference;
    typedef typename Iterator::iterator_category iterator_category;
};

// Specialization for raw pointers (random access)
template <typename T>
struct iterator_traits<T*>
{
    typedef T                             value_type;
    typedef ptrdiff_t                     difference_type;
    typedef T*                            pointer;
    typedef T&                            reference;
    typedef random_access_iterator_tag    iterator_category;
};

// Specialization for const pointers
template <typename T>
struct iterator_traits<const T*>
{
    typedef T                             value_type;
    typedef ptrdiff_t                     difference_type;
    typedef const T*                      pointer;
    typedef const T&                      reference;
    typedef random_access_iterator_tag    iterator_category;
};

// --------------------------- reverse_iterator -------------------------
template <class Iterator>
class reverse_iterator
{
public:
    typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
    typedef typename iterator_traits<Iterator>::value_type        value_type;
    typedef typename iterator_traits<Iterator>::difference_type   difference_type;
    typedef typename iterator_traits<Iterator>::pointer           pointer;
    typedef typename iterator_traits<Iterator>::reference         reference;

    typedef Iterator iterator_type;

    reverse_iterator(void) : _it() {}
    // Direct construction
    explicit reverse_iterator(Iterator it) : _it(it) {}

    // Converting constructor: allows reverse_iterator<iterator> -> reverse_iterator<const_iterator>
    template <class U>
    reverse_iterator(const reverse_iterator<U>& other) : _it(other.base()) {}

    Iterator base(void) const { return _it; }

    reference operator*(void) const
    {
        Iterator tmp = _it;
        return *--tmp;
    }

    pointer operator->(void) const
    {
        Iterator tmp = _it;
        --tmp;
        return &(*tmp);
    }

    reference operator[](difference_type n) const
    {
        return *(_it - n - 1);
    }

    reverse_iterator& operator++(void)
    {
        --_it;
        return *this;
    }

    reverse_iterator& operator--(void)
    {
        ++_it;
        return *this;
    }

    reverse_iterator operator++(int)
    {
        reverse_iterator tmp(*this);
        --_it;
        return tmp;
    }

    reverse_iterator operator--(int)
    {
        reverse_iterator tmp(*this);
        ++_it;
        return tmp;
    }

    reverse_iterator operator+(difference_type n) const
    {
        return reverse_iterator(_it - n);
    }

    reverse_iterator operator-(difference_type n) const
    {
        return reverse_iterator(_it + n);
    }

    difference_type operator-(const reverse_iterator& x) const
    {
        return x._it - _it;
    }

    reverse_iterator& operator+=(difference_type n)
    {
        _it -= n;
        return *this;
    }

    reverse_iterator& operator-=(difference_type n)
    {
        _it += n;
        return *this;
    }

    template <class U> bool operator==(const reverse_iterator<U>& x) const { return _it == x.base(); }
    template <class U> bool operator!=(const reverse_iterator<U>& x) const { return _it != x.base(); }
    template <class U> bool operator<(const reverse_iterator<U>& x) const { return x.base() < _it; }
    template <class U> bool operator>(const reverse_iterator<U>& x) const { return _it < x.base(); }
    template <class U> bool operator<=(const reverse_iterator<U>& x) const { return !(_it < x.base()); }
    template <class U> bool operator>=(const reverse_iterator<U>& x) const { return !(x.base() < _it); }

private:
    Iterator _it;
};

// Les opérateurs de comparaison (==, !=, <, >, <=, >=) sont définis en members
// templates pour supporter la comparaison entre reverse_iterator et
// const_reverse_iterator sans conversion.

// free operator+ (difference_type, reverse_iterator)
template <class Iterator>
reverse_iterator<Iterator> operator+(typename reverse_iterator<Iterator>::difference_type n,
                                     const reverse_iterator<Iterator>& x)
{
    return x + n;
}

} // namespace ft

#endif // ITERATOR_HPP
