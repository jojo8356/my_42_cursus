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

    private:
        vectorIterator(pointer ptr) : _ptr(ptr) {}
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
        : _alloc(alloc), _vct(_alloc.allocate(1)), _size(0), _capacity(1)
    {
    }

    explicit vector(size_type n, const value_type& val = value_type(),
                    const allocator_type& alloc = allocator_type())
        : _alloc(alloc), _vct(_alloc.allocate(n > 0 ? n : 1)), _size(n), _capacity(n > 0 ? n : 1)
    {
        for (size_type i = 0; i < n; ++i)
            _alloc.construct(_vct + i, val);
    }

    template <class InputIterator>
    vector(InputIterator first, InputIterator last,
           const allocator_type& alloc = allocator_type(),
           typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
        : _alloc(alloc), _vct(_alloc.allocate(1)), _size(0), _capacity(1)
    {
        while (first != last)
        {
            push_back(*first);
            ++first;
        }
    }

    vector(const vector& x)
        : _alloc(x._alloc), _vct(_alloc.allocate(x._capacity)), _size(x._size), _capacity(x._capacity)
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
            pointer new_vct = _alloc.allocate(x._capacity);
            for (size_type i = 0; i < _size; ++i)
                _alloc.construct(new_vct + i, _vct[i]);
            for (size_type i = 0; i < _size; ++i)
                _alloc.destroy(_vct + i);
            _alloc.deallocate(_vct, _capacity);
            _vct = new_vct;
            _capacity = x._capacity;
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
                reserve(n);
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
        {
            size_type new_cap = (_capacity == 0) ? 1 : _capacity * 2;
            reserve(new_cap);
        }
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
        // Copy val first: it may be a reference to an element of this vector,
        // which the destroy loop below would invalidate.
        ft::vector<value_type> tmp;
        for (size_type i = 0; i < n; ++i)
            tmp.push_back(val);
        tmp.swap(*this);
        _size = n;
    }

    template <class InputIterator>
    void assign(InputIterator first, InputIterator last,
                typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
    {
        clear();
        while (first != last)
        {
            push_back(*first);
            ++first;
        }
    }

    iterator insert(iterator pos, const value_type& val)
    {
        size_type idx = pos - begin();
        // Build the new content in a temp container, then swap buffers.
        // Copying val first makes it safe even if val is a reference to an
        // element of this very vector (reserve would invalidate it).
        ft::vector<value_type> tmp;
        for (size_type i = 0; i < idx; ++i)
            tmp.push_back(_vct[i]);
        tmp.push_back(val);
        for (size_type i = idx; i < _size; ++i)
            tmp.push_back(_vct[i]);
        tmp.swap(*this);
        return begin() + idx;
    }

    void insert(iterator pos, size_type n, const value_type& val)
    {
        if (n == 0)
            return;
        size_type idx = pos - begin();
        // Build the new content in a temp container, then swap buffers.
        // Copy val n times first so it is safe even if val is a reference to
        // an element of this very vector (reserve would invalidate it).
        ft::vector<value_type> tmp;
        for (size_type i = 0; i < idx; ++i)
            tmp.push_back(_vct[i]);
        for (size_type i = 0; i < n; ++i)
            tmp.push_back(val);
        for (size_type i = idx; i < _size; ++i)
            tmp.push_back(_vct[i]);
        tmp.swap(*this);
    }

    template <class InputIterator>
    iterator insert(iterator pos, InputIterator first, InputIterator last,
                    typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
    {
        size_type idx = pos - begin();
        // Build the new content in a temp container, then swap buffers.
        // We consume first/last exactly once, element by element, without
        // calling distance() (which would advance an input iterator and
        // invalidate it before we read the range).
        ft::vector<value_type> tmp;
        for (size_type i = 0; i < idx; ++i)
            tmp.push_back(_vct[i]);
        for (; first != last; ++first)
            tmp.push_back(*first);
        for (size_type i = idx; i < _size; ++i)
            tmp.push_back(_vct[i]);
        tmp.swap(*this);
        return begin() + idx;
    }

    iterator erase(iterator pos)
    {
        size_type idx = pos - begin();
        for (size_type i = idx; i + 1 < _size; ++i)
        {
            _alloc.destroy(_vct + i);
            _alloc.construct(_vct + i, _vct[i + 1]);
        }
        _alloc.destroy(_vct + _size - 1);
        --_size;
        return begin() + idx;
    }

    iterator erase(iterator first, iterator last)
    {
        size_type f = first - begin();
        size_type l = last - begin();
        size_type count = l - f;
        if (count == 0)
            return begin() + f;
        for (size_type i = 0; f + i + count < _size; ++i)
        {
            _alloc.destroy(_vct + f + i);
            _alloc.construct(_vct + f + i, _vct[f + i + count]);
        }
        for (size_type i = _size - count; i < _size; ++i)
            _alloc.destroy(_vct + i);
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
