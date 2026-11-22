#ifndef DEQUE_HPP
# define DEQUE_HPP

//////////////////////////////////////////////////////////////////////////
//  deque.hpp — ft::deque<T, Alloc> (C++98, circular buffer)
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
class deque
{
public:
    // -------------------------------------------------------- Iterator
    template <bool IsConst>
    class dequeIterator
    {
    public:
        typedef typename ft::conditional<IsConst, const T, T>::type value_type;
        typedef value_type&                               reference;
        typedef value_type*                               pointer;
        typedef ft::ptrdiff_t                             difference_type;
        typedef ft::size_t                                size_type;
        typedef ft::random_access_iterator_tag            iterator_category;

        dequeIterator(void) : _buf(0), _end(0), _ptr(0) {}
        dequeIterator(const dequeIterator& x) : _buf(x._buf), _end(x._end), _ptr(x._ptr) {}
        ~dequeIterator(void) {}

        template <bool B>
        dequeIterator(const dequeIterator<B>& x,
                      typename ft::enable_if<!B>::type* = 0)
            : _buf(x.getBuf()), _end(x.getEnd()), _ptr(x.getPtr())
        {
        }

        dequeIterator& operator=(const dequeIterator& x)
        {
            _buf = x._buf; _end = x._end; _ptr = x._ptr;
            return *this;
        }

        dequeIterator& operator+=(difference_type n)
        {
            _ptr += n;
            while (_ptr >= _end) _ptr -= (_end - _buf);
            while (_ptr < _buf)  _ptr += (_end - _buf);
            return *this;
        }
        dequeIterator& operator-=(difference_type n) { return *this += (-n); }

        template <bool B> bool operator==(const dequeIterator<B>& x) const { return _ptr == x.getPtr(); }
        template <bool B> bool operator!=(const dequeIterator<B>& x) const { return _ptr != x.getPtr(); }

        dequeIterator& operator++(void) { ++_ptr; if (_ptr == _end) _ptr = _buf; return *this; }
        dequeIterator& operator--(void) { if (_ptr == _buf) _ptr = _end; --_ptr; return *this; }
        dequeIterator  operator++(int) { dequeIterator<IsConst> x(*this); ++*this; return x; }
        dequeIterator  operator--(int) { dequeIterator<IsConst> x(*this); --*this; return x; }

        dequeIterator operator+(difference_type n) const { dequeIterator<IsConst> x(*this); x += n; return x; }
        dequeIterator operator-(difference_type n) const { dequeIterator<IsConst> x(*this); x -= n; return x; }

        difference_type operator-(const dequeIterator& x) const
        {
            difference_type cap = _end - _buf;
            difference_type a = _ptr - _buf;
            difference_type b = x._ptr - x._buf;
            difference_type d = a - b;
            if (d > cap / 2) d -= cap;
            else if (d < -cap / 2) d += cap;
            return d;
        }

        template <bool B> bool operator<(const dequeIterator<B>& x) const { return (*this - x.getPtr()) < 0; }
        template <bool B> bool operator>(const dequeIterator<B>& x) const { return x < *this; }
        template <bool B> bool operator<=(const dequeIterator<B>& x) const { return !(x < *this); }
        template <bool B> bool operator>=(const dequeIterator<B>& x) const { return !(*this < x); }

        reference operator[](size_type n) const { return (*(_ptr + (difference_type)n)); }
        reference operator*(void) const { return *_ptr; }
        pointer   operator->(void) const { return _ptr; }

        pointer  getPtr(void) const { return _ptr; }
        pointer  getBuf(void) const { return _buf; }
        pointer  getEnd(void) const { return _end; }

        friend dequeIterator operator+(difference_type n, const dequeIterator& x) { return x + n; }

    private:
        typename ft::conditional<IsConst, const T*, T*>::type _buf;
        typename ft::conditional<IsConst, const T*, T*>::type _end;
        typename ft::conditional<IsConst, const T*, T*>::type _ptr;

        dequeIterator(typename ft::conditional<IsConst, const T*, T*>::type buf,
                      typename ft::conditional<IsConst, const T*, T*>::type end,
                      typename ft::conditional<IsConst, const T*, T*>::type ptr)
            : _buf(buf), _end(end), _ptr(ptr) {}

        friend class deque;
        friend class reverse_iterator<dequeIterator>;
    };

    // ------------------------------------------------------- Member types
    typedef T                                   value_type;
    typedef Alloc                               allocator_type;
    typedef typename allocator_type::reference  reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::pointer     pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef dequeIterator<false>                iterator;
    typedef dequeIterator<true>                 const_iterator;
    typedef ft::reverse_iterator<iterator>      reverse_iterator;
    typedef ft::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef typename iterator::difference_type difference_type;
    typedef typename iterator::size_type       size_type;

    // ------------------------------------------------------- Constructors
    explicit deque(const allocator_type& alloc = allocator_type())
        : _alloc(alloc), _buf(0), _end(0), _head(0), _size(0), _cap(0)
    {
    }

    explicit deque(size_type n, const value_type& val = value_type(),
                   const allocator_type& alloc = allocator_type())
        : _alloc(alloc), _buf(0), _end(0), _head(0), _size(0), _cap(0)
    {
        for (size_type i = 0; i < n; ++i)
            push_back(val);
    }

    template <class InputIterator>
    deque(InputIterator first, InputIterator last,
          const allocator_type& alloc = allocator_type(),
          typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
        : _alloc(alloc), _buf(0), _end(0), _head(0), _size(0), _cap(0)
    {
        while (first != last)
        {
            push_back(*first);
            ++first;
        }
    }

    deque(const deque& x)
        : _alloc(x._alloc), _buf(0), _end(0), _head(0), _size(0), _cap(0)
    {
        for (size_type i = 0; i < x._size; ++i)
            push_back(x._at(i));
    }

    ~deque(void)
    {
        clear();
        if (_buf)
            _alloc.deallocate(_buf, _cap);
    }

    // ------------------------------------------------------- Assignment
    deque& operator=(const deque& x)
    {
        if (this == &x)
            return *this;
        clear();
        for (size_type i = 0; i < x._size; ++i)
            push_back(x._at(i));
        return *this;
    }

    // ------------------------------------------------------- Iterators
    iterator begin(void)
    {
        return _size == 0 ? iterator() : iterator(_buf, _buf + _cap, _buf + _head);
    }
    const_iterator begin(void) const
    {
        return _size == 0 ? const_iterator() : const_iterator(_buf, _buf + _cap, _buf + _head);
    }
    iterator end(void)
    {
        if (_size == 0)
            return iterator();
        return iterator(_buf, _buf + _cap, _buf + (_head + _size) % _cap);
    }
    const_iterator end(void) const
    {
        if (_size == 0)
            return const_iterator();
        return const_iterator(_buf, _buf + _cap, _buf + (_head + _size) % _cap);
    }

    reverse_iterator rbegin(void) { return reverse_iterator(end()); }
    const_reverse_iterator rbegin(void) const { return const_reverse_iterator(end()); }
    reverse_iterator rend(void) { return reverse_iterator(begin()); }
    const_reverse_iterator rend(void) const { return const_reverse_iterator(begin()); }

    // ------------------------------------------------------- Capacity
    size_type size(void) const { return _size; }
    bool      empty(void) const { return _size == 0; }
    size_type max_size(void) const { return _alloc.max_size(); }

    // ------------------------------------------------------- Element access
    reference       operator[](size_type n) { return _at(n); }
    const_reference operator[](size_type n) const { return _at(n); }

    reference       at(size_type n)
    {
        if (n >= _size)
            throw ft::out_of_range("deque::at");
        return _at(n);
    }
    const_reference at(size_type n) const
    {
        if (n >= _size)
            throw ft::out_of_range("deque::at");
        return _at(n);
    }

    reference       front(void) { return _at(0); }
    const_reference front(void) const { return _at(0); }
    reference       back(void) { return _at(_size - 1); }
    const_reference back(void) const { return _at(_size - 1); }

    // ------------------------------------------------------- Modifiers
    void push_back(const value_type& val)
    {
        if (_size == _cap)
            _grow();
        _alloc.construct(_buf + (_head + _size) % _cap, val);
        ++_size;
    }

    void push_front(const value_type& val)
    {
        if (_size == _cap)
            _grow();
        if (_head == 0)
        {
            _head = _cap - 1;
        }
        else
        {
            --_head;
        }
        _alloc.construct(_buf + _head, val);
        ++_size;
    }

    void pop_back(void)
    {
        _alloc.destroy(_buf + (_head + _size - 1) % _cap);
        --_size;
    }

    void pop_front(void)
    {
        _alloc.destroy(_buf + _head);
        _head = (_head + 1) % _cap;
        --_size;
    }

    iterator insert(iterator pos, const value_type& val)
    {
        size_type idx = pos - begin();
        if (idx == _size)
        {
            push_back(val);
            return begin() + idx;
        }
        if (idx == 0)
        {
            push_front(val);
            return begin();
        }
        // shift [idx, size) right by one
        for (size_type i = _size; i > idx; --i)
            _construct_at(i, _at(i - 1));
        _construct_at(idx, val);
        ++_size;
        return begin() + idx;
    }

    void insert(iterator pos, size_type n, const value_type& val)
    {
        for (size_type i = 0; i < n; ++i)
            insert(pos, val);
    }

    template <class InputIterator>
    iterator insert(iterator pos, InputIterator first, InputIterator last,
                    typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
    {
        size_type idx = pos - begin();
        while (first != last)
        {
            insert(begin() + idx, *first);
            ++first;
            ++idx;
        }
        return begin() + (idx - (first != last ? 1 : 0));
    }

    iterator erase(iterator pos)
    {
        size_type idx = pos - begin();
        for (size_type i = idx; i + 1 < _size; ++i)
            _construct_at(i, _at(i + 1));
        _alloc.destroy(_buf + (_head + _size - 1) % _cap);
        --_size;
        return begin() + idx;
    }

    iterator erase(iterator first, iterator last)
    {
        size_type f = first - begin();
        size_type l = last - begin();
        size_type count = l - f;
        for (size_type i = 0; f + i + count < _size; ++i)
            _construct_at(f + i, _at(f + i + count));
        for (size_type i = _size - count; i < _size; ++i)
            _alloc.destroy(_buf + (_head + i) % _cap);
        _size -= count;
        return begin() + f;
    }

    void resize(size_type n, const value_type& val = value_type())
    {
        if (n < _size)
        {
            while (_size > n)
                pop_back();
        }
        else
        {
            while (_size < n)
                push_back(val);
        }
    }

    void assign(size_type n, const value_type& val)
    {
        clear();
        for (size_type i = 0; i < n; ++i)
            push_back(val);
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

    void clear(void)
    {
        for (size_type i = 0; i < _size; ++i)
            _alloc.destroy(_buf + (_head + i) % _cap);
        _size = 0;
        _head = 0;
    }

    void swap(deque& x)
    {
        if (this == &x)
            return;
        ft::swap(_buf, x._buf);
        ft::swap(_end, x._end);
        ft::swap(_head, x._head);
        ft::swap(_size, x._size);
        ft::swap(_cap, x._cap);
    }

private:
    reference _at(size_type n) const
    {
        return _buf[(_head + n) % _cap];
    }

    void _construct_at(size_type logical_pos, const value_type& val)
    {
        size_type pos = (_head + logical_pos) % _cap;
        _alloc.construct(_buf + pos, val);
    }

    void _grow(void)
    {
        size_type new_cap = (_cap == 0) ? 2 : _cap * 2 + 2;
        pointer new_buf = _alloc.allocate(new_cap);
        // copy elements to the middle
        for (size_type i = 0; i < _size; ++i)
            _alloc.construct(new_buf + new_cap / 2 + i, _at(i));
        for (size_type i = 0; i < _size; ++i)
            _alloc.destroy(_buf + (_head + i) % _cap);
        if (_buf)
            _alloc.deallocate(_buf, _cap);
        _buf = new_buf;
        _end = new_buf + new_cap;
        _head = new_cap / 2;
        _cap = new_cap;
    }

    allocator_type _alloc;
    pointer     _buf;
    pointer     _end;
    size_type   _head;
    size_type   _size;
    size_type   _cap;
};

// ----------------------------------------------------- relational ops
template <class T, class Alloc>
bool operator==(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs)
{
    return lhs.size() == rhs.size() && ft::equal(lhs.begin(), lhs.end(), rhs.begin());
}
template <class T, class Alloc>
bool operator!=(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs) { return !(lhs == rhs); }
template <class T, class Alloc>
bool operator<(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs)
{
    return ft::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <class T, class Alloc>
bool operator>(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs) { return rhs < lhs; }
template <class T, class Alloc>
bool operator<=(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs) { return !(rhs < lhs); }
template <class T, class Alloc>
bool operator>=(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs) { return !(lhs < rhs); }

template <class T, class Alloc>
void swap(deque<T, Alloc>& lhs, deque<T, Alloc>& rhs) { lhs.swap(rhs); }

} // namespace ft

#endif // DEQUE_HPP
