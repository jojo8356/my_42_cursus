#ifndef VECTOR_HPP
# define VECTOR_HPP

//////////////////////////////////////////////////////////////////////////
//  vector.hpp — ft::vector<T, Alloc> (C++98, dynamic array)
//////////////////////////////////////////////////////////////////////////

# include <memory>
# include <stdexcept>
# include "type_traits.hpp"
# include "exception.hpp"
# include "iterator.hpp"
# include "utility.hpp"
# include "algorithm.hpp"

namespace ft
{

template <class T, class Alloc = std::allocator<T> >
class vector
{
public:
    // -------------------------------------------------------- Iterator
    template <bool IsConst>
    class vectorIterator
    {
    public:
        typedef typename ft::conditional<IsConst, const T, T>::type value_type;
        typedef value_type&                               reference;
        typedef value_type*                               pointer;
        typedef ft::ptrdiff_t                             difference_type;
        typedef ft::size_t                                size_type;
        typedef ft::random_access_iterator_tag             iterator_category;

        vectorIterator(void) : _ptr(0) {}
        vectorIterator(const vectorIterator& x) : _ptr(x._ptr) {}
        ~vectorIterator(void) {}

        template <bool B>
        vectorIterator(const vectorIterator<B>& x,
                       typename ft::enable_if<!B>::type* = 0)
            : _ptr(x.getPtr())
        {
        }

        vectorIterator& operator=(const vectorIterator& x)
        {
            _ptr = x._ptr;
            return *this;
        }

        vectorIterator& operator+=(difference_type n)
        {
            _ptr += n;
            return *this;
        }
        vectorIterator& operator-=(difference_type n)
        {
            _ptr -= n;
            return *this;
        }

        // Comparison
        template <bool B> bool operator==(const vectorIterator<B>& x) const { return _ptr == x.getPtr(); }
        template <bool B> bool operator!=(const vectorIterator<B>& x) const { return _ptr != x.getPtr(); }
        template <bool B> bool operator<(const vectorIterator<B>& x) const { return _ptr < x.getPtr(); }
        template <bool B> bool operator>(const vectorIterator<B>& x) const { return _ptr > x.getPtr(); }
        template <bool B> bool operator<=(const vectorIterator<B>& x) const { return _ptr <= x.getPtr(); }
        template <bool B> bool operator>=(const vectorIterator<B>& x) const { return _ptr >= x.getPtr(); }

        // Incrementation
        vectorIterator& operator++(void) { ++_ptr; return *this; }
        vectorIterator& operator--(void) { --_ptr; return *this; }
        vectorIterator  operator++(int) { vectorIterator<IsConst> x(*this); ++_ptr; return x; }
        vectorIterator  operator--(int) { vectorIterator<IsConst> x(*this); --_ptr; return x; }

        // Arithmetic
        vectorIterator operator+(difference_type n) const { return vectorIterator<IsConst>(_ptr + n); }
        vectorIterator operator-(difference_type n) const { return vectorIterator<IsConst>(_ptr - n); }
        difference_type operator-(const vectorIterator& x) const { return _ptr - x._ptr; }

        // Dereference
        reference operator[](size_type n) const { return *(_ptr + n); }
        reference operator*(void) const { return *_ptr; }
        pointer   operator->(void) const { return _ptr; }

        pointer getPtr(void) const { return _ptr; }

        friend vectorIterator operator+(difference_type n, const vectorIterator& x)
        {
            return vectorIterator<IsConst>(x.getPtr() + n);
        }

        vectorIterator(pointer ptr) : _ptr(ptr) {}

    private:
        pointer _ptr;

        friend class vector;
        friend class reverse_iterator<vectorIterator>;
    };

    // ------------------------------------------------------- Member types
    typedef T                                    value_type;
    typedef Alloc                                allocator_type;
    typedef typename allocator_type::reference   reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::pointer     pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef vectorIterator<false>                iterator;
    typedef vectorIterator<true>                 const_iterator;
    typedef ft::reverse_iterator<iterator>       reverse_iterator;
    typedef ft::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef typename iterator::difference_type   difference_type;
    typedef typename iterator::size_type         size_type;

    // ------------------------------------------------------- Constructors
    explicit vector(const allocator_type& alloc = allocator_type())
        : _alloc(alloc), _vct(_alloc.allocate(0)), _size(0), _capacity(0)
    {
    }

    explicit vector(size_type n, const value_type& val = value_type(),
                    const allocator_type& alloc = allocator_type())
        : _alloc(alloc), _vct(_alloc.allocate(n)), _size(n), _capacity(n)
    {
        for (size_type i = 0; i < n; ++i)
            _alloc.construct(_vct + i, val);
    }

    template <class InputIterator>
    vector(InputIterator first, InputIterator last,
           const allocator_type& alloc = allocator_type(),
           typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
        : _alloc(alloc), _vct(_alloc.allocate(0)), _size(0), _capacity(0)
    {
        _init_from_range(first, last, _ra_tag(first));
    }

    template <class InputIterator>
    void _init_from_range(InputIterator first, InputIterator last, ft::true_type)
    {
        size_type n = last - first;
        reserve(n);
        for (size_type i = 0; i < n; ++i)
        {
            _alloc.construct(_vct + i, *first);
            ++first;
        }
        _size = n;
    }

    template <class InputIterator>
    void _init_from_range(InputIterator first, InputIterator last, ft::false_type)
    {
        while (first != last)
        {
            push_back(*first);
            ++first;
        }
    }

    vector(const vector& x)
        : _alloc(x._alloc), _vct(_alloc.allocate(x._size)), _size(x._size), _capacity(x._size)
    {
        for (size_type i = 0; i < _size; ++i)
            _alloc.construct(_vct + i, x._vct[i]);
    }

    // ------------------------------------------------------- Destructor
    ~vector(void)
    {
        for (size_type i = 0; i < _size; ++i)
            _alloc.destroy(_vct + i);
        _alloc.deallocate(_vct, _capacity);
    }

    // ------------------------------------------------------- Assignment
    vector& operator=(const vector& x)
    {
        if (this == &x)
            return *this;

        if (x._size > _capacity)
        {
            pointer new_vct = _alloc.allocate(x._size);
            for (size_type i = 0; i < _size; ++i)
                _alloc.construct(new_vct + i, _vct[i]);
            for (size_type i = 0; i < _size; ++i)
                _alloc.destroy(_vct + i);
            _alloc.deallocate(_vct, _capacity);
            _vct = new_vct;
            _capacity = x._size;
        }

        for (size_type i = 0; i < _size; ++i)
            _alloc.destroy(_vct + i);

        _size = x._size;
        for (size_type i = 0; i < _size; ++i)
            _alloc.construct(_vct + i, x._vct[i]);

        return *this;
    }

    // ------------------------------------------------------- Iterators
    iterator begin(void) { return iterator(_vct); }
    const_iterator begin(void) const { return const_iterator(_vct); }
    iterator end(void) { return iterator(_vct + _size); }
    const_iterator end(void) const { return const_iterator(_vct + _size); }

    reverse_iterator rbegin(void) { return reverse_iterator(end()); }
    const_reverse_iterator rbegin(void) const { return const_reverse_iterator(end()); }
    reverse_iterator rend(void) { return reverse_iterator(begin()); }
    const_reverse_iterator rend(void) const { return const_reverse_iterator(begin()); }

    // ------------------------------------------------------- Capacity
    size_type size(void) const { return _size; }
    size_type capacity(void) const { return _capacity; }
    bool      empty(void) const { return _size == 0; }
    size_type max_size(void) const { return _alloc.max_size(); }

    void reserve(size_type n)
    {
        if (n > _alloc.max_size())
            throw std::length_error("vector::reserve");
        if (n > _capacity)
        {
            pointer new_vct = _alloc.allocate(n);
            for (size_type i = 0; i < _size; ++i)
            {
                _alloc.construct(new_vct + i, _vct[i]);
                _alloc.destroy(_vct + i);
            }
            _alloc.deallocate(_vct, _capacity);
            _vct = new_vct;
            _capacity = n;
        }
    }

    void resize(size_type n, const value_type& val = value_type())
    {
        if (n < _size)
        {
            for (size_type i = n; i < _size; ++i)
                _alloc.destroy(_vct + i);
        }
        else if (n > _size)
        {
            if (n > _capacity)
                reserve(_check_len(n - _size)); // libstdc++: _M_default_append -> _M_check_len(n-size)
            for (size_type i = _size; i < n; ++i)
                _alloc.construct(_vct + i, val);
        }
        _size = n;
    }

    // ------------------------------------------------------- Element access
    reference       operator[](size_type n) { return _vct[n]; }
    const_reference operator[](size_type n) const { return _vct[n]; }

    reference       at(size_type n)
    {
        if (n >= _size)
            throw ft::out_of_range("vector::at");
        return _vct[n];
    }
    const_reference at(size_type n) const
    {
        if (n >= _size)
            throw ft::out_of_range("vector::at");
        return _vct[n];
    }

    reference       front(void) { return _vct[0]; }
    const_reference front(void) const { return _vct[0]; }
    reference       back(void) { return _vct[_size - 1]; }
    const_reference back(void) const { return _vct[_size - 1]; }

    // ------------------------------------------------------- Modifiers
    void push_back(const value_type& val)
    {
        if (_size >= _capacity)
            reserve(_check_len(1));
        _alloc.construct(_vct + _size, val);
        ++_size;
    }

    void pop_back(void)
    {
        _alloc.destroy(_vct + _size - 1);
        --_size;
    }

    void assign(size_type n, const value_type& val)
    {
        // Copy val first: it may be a reference to an element of this vector.
        value_type tmp_val(val);
        if (n > _capacity)
            reserve(n); // libstdc++ alloue exactement n (vector tmp(n, val) puis swap)
        for (size_type i = 0; i < _size; ++i)
            _alloc.destroy(_vct + i);
        _size = 0;
        for (size_type i = 0; i < n; ++i)
            _alloc.construct(_vct + i, tmp_val);
        _size = n;
    }

    template <class InputIterator>
    void assign(InputIterator first, InputIterator last,
                typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
    {
        for (size_type i = 0; i < _size; ++i)
            _alloc.destroy(_vct + i);
        _size = 0;
        _assign_from_range(first, last, _ra_tag(first));
    }

    template <class InputIterator>
    void _assign_from_range(InputIterator first, InputIterator last, ft::true_type)
    {
        size_type n = last - first;
        reserve(n);
        for (size_type i = 0; i < n; ++i)
        {
            _alloc.construct(_vct + i, *first);
            ++first;
        }
        _size = n;
    }

    template <class InputIterator>
    void _assign_from_range(InputIterator first, InputIterator last, ft::false_type)
    {
        while (first != last)
        {
            push_back(*first);
            ++first;
        }
    }

    iterator insert(iterator pos, const value_type& val)
    {
        size_type idx = pos - begin();
        // Copy val first: it may alias an element of this vector.
        value_type tmp_val(val);
        if (_size >= _capacity)
            reserve(_check_len(1));
        _shift_right(idx, 1, integral_constant<is_trivially_copyable<T>::value>());
        _alloc.construct(_vct + idx, tmp_val);
        ++_size;
        return begin() + idx;
    }

    void insert(iterator pos, size_type n, const value_type& val)
    {
        if (n == 0)
            return;
        size_type idx = pos - begin();
        value_type tmp_val(val);
        if (_size + n > _capacity)
            reserve(_check_len(n));
        _shift_right(idx, n, integral_constant<is_trivially_copyable<T>::value>());
        for (size_type i = 0; i < n; ++i)
            _alloc.construct(_vct + idx + i, tmp_val);
        _size += n;
    }

    template <class InputIterator>
    void insert(iterator pos, InputIterator first, InputIterator last,
                typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
    {
        _insert_range_impl(pos, first, last, _ra_tag(first));
    }

    template <class InputIterator>
    void _insert_range_impl(iterator pos, InputIterator first, InputIterator last, ft::true_type)
    {
        size_type idx = pos - begin();
        size_type n = last - first;
        if (_size + n > _capacity)
            reserve(_check_len(n));
        _shift_right(idx, n, integral_constant<is_trivially_copyable<T>::value>());
        for (size_type i = 0; i < n; ++i)
        {
            _alloc.construct(_vct + idx + i, *first);
            ++first;
        }
        _size += n;
    }

    template <class InputIterator>
    void _insert_range_impl(iterator pos, InputIterator first, InputIterator last, ft::false_type)
    {
        ft::vector<value_type, Alloc> tmp;
        for (; first != last; ++first)
            tmp.push_back(*first);
        insert(pos, tmp.begin(), tmp.end());
    }

    iterator erase(iterator pos)
    {
        size_type idx = pos - begin();
        _alloc.destroy(_vct + idx);
        _shift_left(idx, 1, integral_constant<is_trivially_copyable<T>::value>());
        _size -= (_size > 0) ? 1 : 0;
        return begin() + idx;
    }

    iterator erase(iterator first, iterator last)
    {
        size_type f = first - begin();
        size_type l = last - begin();
        size_type count = l - f;
        if (count == 0)
            return begin() + f;
        for (size_type i = f; i < f + count; ++i)
            _alloc.destroy(_vct + i);
        _shift_left(f, count, integral_constant<is_trivially_copyable<T>::value>());
        _size -= count;
        return begin() + f;
    }

    void clear(void)
    {
        for (size_type i = 0; i < _size; ++i)
            _alloc.destroy(_vct + i);
        _size = 0;
    }

    void swap(vector& x)
    {
        if (this == &x)
            return;
        ft::swap(_vct, x._vct);
        ft::swap(_size, x._size);
        ft::swap(_capacity, x._capacity);
        // allocators are not swapped (they must be equal in standard containers)
    }

private:
    // Croissance de capacite de libstdc++ : size + max(size, n)
    size_type _check_len(size_type n) const
    {
        size_type m = (_size >= n) ? _size : n;
        return _size + m;
    }

    template <class It>
    static bool _is_random_access(It)
    {
        typedef typename ft::iterator_traits<It>::iterator_category cat;
        return ft::is_same<cat, ft::random_access_iterator_tag>::value;
    }

    template <class It>
    static ft::integral_constant<ft::is_same<typename ft::iterator_traits<It>::iterator_category, ft::random_access_iterator_tag>::value>
    _ra_tag(It)
    {
        typedef typename ft::iterator_traits<It>::iterator_category cat;
        return ft::integral_constant<ft::is_same<cat, ft::random_access_iterator_tag>::value>();
    }

    // Shift [idx, _size) right by n. Trivially-copyable types use raw
    // assignment (destinations may be raw memory); others construct the
    // destination from the source and destroy the source.
    void _shift_right(size_type idx, size_type n, ft::true_type)
    {
        if (n == 0)
            return;
        for (size_type i = _size; i > idx; --i)
            _vct[i + n - 1] = _vct[i - 1];
    }
    void _shift_right(size_type idx, size_type n, ft::false_type)
    {
        if (n == 0)
            return;
        for (size_type i = _size; i > idx; --i)
        {
            _alloc.construct(_vct + i + n - 1, _vct[i - 1]);
            _alloc.destroy(_vct + i - 1);
        }
    }

    // Shift [idx+count, _size) left by count into [idx, _size-count).
    void _shift_left(size_type idx, size_type count, ft::true_type)
    {
        if (count == 0)
            return;
        for (size_type i = idx; i + count < _size; ++i)
            _vct[i] = _vct[i + count];
    }
    void _shift_left(size_type idx, size_type count, ft::false_type)
    {
        if (count == 0)
            return;
        for (size_type i = idx; i + count < _size; ++i)
        {
            _alloc.construct(_vct + i, _vct[i + count]);
            _alloc.destroy(_vct + i + count);
        }
    }

    allocator_type _alloc;
    pointer     _vct;
    size_type   _size;
    size_type   _capacity;
};

// ----------------------------------------------------- relational ops
template <class T, class Alloc>
bool operator==(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs)
{
    return lhs.size() == rhs.size() && ft::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class T, class Alloc>
bool operator!=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class Alloc>
bool operator<(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs)
{
    return ft::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T, class Alloc>
bool operator>(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs)
{
    return rhs < lhs;
}

template <class T, class Alloc>
bool operator<=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs)
{
    return !(rhs < lhs);
}

template <class T, class Alloc>
bool operator>=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs)
{
    return !(lhs < rhs);
}

template <class T, class Alloc>
void swap(vector<T, Alloc>& lhs, vector<T, Alloc>& rhs)
{
    lhs.swap(rhs);
}

} // namespace ft

#endif // VECTOR_HPP
