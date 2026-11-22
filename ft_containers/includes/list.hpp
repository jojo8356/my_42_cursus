#ifndef LIST_HPP
# define LIST_HPP

//////////////////////////////////////////////////////////////////////////
//  list.hpp — ft::list<T, Alloc> (C++98, circular doubly-linked list)
//////////////////////////////////////////////////////////////////////////

# include <memory>
# include <functional>
# include <stdexcept>
# include "type_traits.hpp"
# include "iterator.hpp"
# include "utility.hpp"
# include "algorithm.hpp"
# include "vector.hpp"

namespace ft
{

template <class T, class Alloc = std::allocator<T> >
class list
{
public:
private:
    // ------------------------------------------------------- Node
    struct node
    {
        T        data;
        node*    next;
        node*    prev;

        node(const T& val) : data(val), next(0), prev(0) {}
        node(void) : data(), next(0), prev(0) {}
    };

    typedef typename Alloc::template rebind<node>::other node_allocator_type;
    typedef typename node_allocator_type::pointer      node_ptr;
    typedef typename node_allocator_type::reference    node_reference;

public:
    // -------------------------------------------------------- Iterator
    template <bool IsConst>
    class listIterator
    {
    public:
        typedef typename ft::conditional<IsConst, const T, T>::type value_type;
        typedef value_type&                                       reference;
        typedef value_type*                                       pointer;
        typedef ft::ptrdiff_t                                     difference_type;
        typedef ft::size_t                                       size_type;
        typedef ft::bidirectional_iterator_tag                    iterator_category;

        typedef node_allocator_type node_allocator;
        typedef typename node_allocator::pointer node_ptr;

        listIterator(void) : _node(0) {}
        listIterator(const listIterator& x) : _node(x._node) {}
        ~listIterator(void) {}

        template <bool B>
        listIterator(const listIterator<B>& x,
                     typename ft::enable_if<!B>::type* = 0)
            : _node(x.getNode())
        {
        }

        listIterator& operator=(const listIterator& x)
        {
            _node = x._node;
            return *this;
        }

        // Comparison
        template <bool B> bool operator==(const listIterator<B>& x) const { return _node == x.getNode(); }
        template <bool B> bool operator!=(const listIterator<B>& x) const { return _node != x.getNode(); }

        // Incrementation
        listIterator& operator++(void) { _node = _node->next; return *this; }
        listIterator& operator--(void) { _node = _node->prev; return *this; }
        listIterator  operator++(int) { listIterator<IsConst> x(*this); _node = _node->next; return x; }
        listIterator  operator--(int) { listIterator<IsConst> x(*this); _node = _node->prev; return x; }

        // Dereference
        reference operator*(void) const { return _node->data; }
        pointer   operator->(void) const { return &(_node->data); }

        node_ptr getNode(void) const { return _node; }

    private:
        explicit listIterator(node_ptr node) : _node(node) {}
        node_ptr _node;

        friend class list;
        friend class reverse_iterator<listIterator>;
    };

    // ------------------------------------------------------- Member types
    typedef T                                   value_type;
    typedef Alloc                               allocator_type;
    typedef typename allocator_type::reference  reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef listIterator<false>                iterator;
    typedef listIterator<true>                 const_iterator;
    typedef ft::reverse_iterator<iterator>     reverse_iterator;
    typedef ft::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef typename iterator::difference_type difference_type;
    typedef typename iterator::size_type       size_type;

    // ------------------------------------------------------- Constructors
    explicit list(const allocator_type& alloc = allocator_type())
        : _alloc(alloc), _size(0)
    {
        _sentinel = _node_alloc.allocate(1);
        _node_alloc.construct(_sentinel, node());
        _sentinel->next = _sentinel;
        _sentinel->prev = _sentinel;
    }

    explicit list(size_type n, const value_type& val = value_type(),
                  const allocator_type& alloc = allocator_type())
        : _alloc(alloc), _size(0)
    {
        _sentinel = _node_alloc.allocate(1);
        _node_alloc.construct(_sentinel, node());
        _sentinel->next = _sentinel;
        _sentinel->prev = _sentinel;
        for (size_type i = 0; i < n; ++i)
            push_back(val);
    }

    template <class InputIterator>
    list(InputIterator first, InputIterator last,
         const allocator_type& alloc = allocator_type(),
         typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
        : _alloc(alloc), _size(0)
    {
        _sentinel = _node_alloc.allocate(1);
        _node_alloc.construct(_sentinel, node());
        _sentinel->next = _sentinel;
        _sentinel->prev = _sentinel;
        while (first != last)
        {
            push_back(*first);
            ++first;
        }
    }

    list(const list& x)
        : _alloc(x._alloc), _size(0)
    {
        _sentinel = _node_alloc.allocate(1);
        _node_alloc.construct(_sentinel, node());
        _sentinel->next = _sentinel;
        _sentinel->prev = _sentinel;
        for (const_iterator it = x.begin(); it != x.end(); ++it)
            push_back(*it);
    }

    ~list(void)
    {
        clear();
        _node_alloc.destroy(_sentinel);
        _node_alloc.deallocate(_sentinel, 1);
    }

    // ------------------------------------------------------- Assignment
    list& operator=(const list& x)
    {
        if (this == &x)
            return *this;
        clear();
        for (const_iterator it = x.begin(); it != x.end(); ++it)
            push_back(*it);
        return *this;
    }

    // ------------------------------------------------------- Iterators
    iterator begin(void) { return iterator(_sentinel->next); }
    const_iterator begin(void) const { return const_iterator(_sentinel->next); }
    iterator end(void) { return iterator(_sentinel); }
    const_iterator end(void) const { return const_iterator(_sentinel); }

    reverse_iterator rbegin(void) { return reverse_iterator(end()); }
    const_reverse_iterator rbegin(void) const { return const_reverse_iterator(end()); }
    reverse_iterator rend(void) { return reverse_iterator(begin()); }
    const_reverse_iterator rend(void) const { return const_reverse_iterator(begin()); }

    // ------------------------------------------------------- Capacity
    size_type size(void) const { return _size; }
    bool      empty(void) const { return _size == 0; }
    size_type max_size(void) const { return _alloc.max_size(); }

    // ------------------------------------------------------- Element access
    reference       front(void) { return _sentinel->next->data; }
    const_reference front(void) const { return _sentinel->next->data; }
    reference       back(void) { return _sentinel->prev->data; }
    const_reference back(void) const { return _sentinel->prev->data; }

    // ------------------------------------------------------- Modifiers
    void push_back(const value_type& val)
    {
        node_ptr n = _node_alloc.allocate(1);
        _node_alloc.construct(n, node(val));
        n->prev = _sentinel->prev;
        n->next = _sentinel;
        _sentinel->prev->next = n;
        _sentinel->prev = n;
        ++_size;
    }

    void push_front(const value_type& val)
    {
        node_ptr n = _node_alloc.allocate(1);
        _node_alloc.construct(n, node(val));
        n->next = _sentinel->next;
        n->prev = _sentinel;
        _sentinel->next->prev = n;
        _sentinel->next = n;
        ++_size;
    }

    void pop_back(void)
    {
        node_ptr n = _sentinel->prev;
        n->prev->next = _sentinel;
        _sentinel->prev = n->prev;
        _node_alloc.destroy(n);
        _node_alloc.deallocate(n, 1);
        --_size;
    }

    void pop_front(void)
    {
        node_ptr n = _sentinel->next;
        n->next->prev = _sentinel;
        _sentinel->next = n->next;
        _node_alloc.destroy(n);
        _node_alloc.deallocate(n, 1);
        --_size;
    }

    iterator insert(iterator pos, const value_type& val)
    {
        node_ptr n = _node_alloc.allocate(1);
        _node_alloc.construct(n, node(val));
        node_ptr p = pos.getNode();
        n->prev = p->prev;
        n->next = p;
        p->prev->next = n;
        p->prev = n;
        ++_size;
        return iterator(n);
    }

    void insert(iterator pos, size_type n, const value_type& val)
    {
        for (size_type i = 0; i < n; ++i)
            insert(pos, val);
    }

    template <class InputIterator>
    void insert(iterator pos, InputIterator first, InputIterator last,
                typename ft::enable_if<!ft::is_same<InputIterator, int>::value>::type* = 0)
    {
        while (first != last)
        {
            insert(pos, *first);
            ++first;
        }
    }

    iterator erase(iterator pos)
    {
        node_ptr n = pos.getNode();
        n->prev->next = n->next;
        n->next->prev = n->prev;
        node_ptr ret = n->next;
        _node_alloc.destroy(n);
        _node_alloc.deallocate(n, 1);
        --_size;
        return iterator(ret);
    }

    iterator erase(iterator first, iterator last)
    {
        while (first != last)
            first = erase(first);
        return last;
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

    void clear(void)
    {
        while (!empty())
            pop_front();
    }

    void swap(list& x)
    {
        if (this == &x)
            return;
        ft::swap(_sentinel, x._sentinel);
        ft::swap(_size, x._size);
    }

    // ------------------------------------------------------- List algorithms
    void sort(void)
    {
        sort(std::less<value_type>());
    }

    template <class Predicate>
    void sort(Predicate pred)
    {
        if (_size < 2)
            return;
        // Copy the values out, do a stable insertion sort on them, then
        // write them back. Modifying the list in place while iterating is
        // unsafe (reinsertion invalidates the iteration), so we sort a
        // temporary buffer and assign.
        ft::vector<value_type> tmp;
        for (iterator it = begin(); it != end(); ++it)
            tmp.push_back(*it);
        // stable insertion sort on tmp
        for (size_type i = 1; i < tmp.size(); ++i)
        {
            value_type key = tmp[i];
            size_type j = i;
            while (j > 0 && pred(key, tmp[j - 1]))
            {
                tmp[j] = tmp[j - 1];
                --j;
            }
            tmp[j] = key;
        }
        clear();
        for (size_type i = 0; i < tmp.size(); ++i)
            push_back(tmp[i]);
    }

    void reverse(void)
    {
        if (_size < 2)
            return;
        node_ptr cur = _sentinel;
        do
        {
            node_ptr tmp = cur->next;
            cur->next = cur->prev;
            cur->prev = tmp;
            cur = tmp;
        } while (cur != _sentinel);
    }

    void unique(void)
    {
        if (_size < 2)
            return;
        iterator it = begin();
        iterator prev = it++;
        while (it != end())
        {
            if (*it == *prev)
                it = erase(it);
            else
            {
                prev = it;
                ++it;
            }
        }
    }

    template <class Predicate>
    void unique(Predicate pred)
    {
        if (_size < 2)
            return;
        iterator it = begin();
        iterator prev = it++;
        while (it != end())
        {
            if (pred(*prev, *it))
                it = erase(it);
            else
            {
                prev = it;
                ++it;
            }
        }
    }

    void assign(size_type n, const T& val)
    {
        clear();
        for (size_type i = 0; i < n; ++i)
            insert(end(), val);
    }

    template <class InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        clear();
        for (; first != last; ++first)
            insert(end(), *first);
    }

    void remove(const T& value)
    {
        iterator it = begin();
        while (it != end())
        {
            if (*it == value)
                it = erase(it);
            else
                ++it;
        }
    }

    template <class Predicate>
    void remove_if(Predicate pred)
    {
        iterator it = begin();
        while (it != end())
        {
            if (pred(*it))
                it = erase(it);
            else
                ++it;
        }
    }

    void merge(list& x)
    {
        if (this == &x)
            return;
        iterator i = begin();
        iterator j = x.begin();
        while (i != end() && j != x.end())
        {
            if (!(*j < *i))
                ++i;
            else
            {
                iterator next = j;
                ++next;
                i = insert(i, *j);
                j = next;
                ++i;
            }
        }
        while (j != x.end())
        {
            i = insert(i, *j);
            ++j;
            ++i;
        }
        x.clear();
    }

    template <class Predicate>
    void merge(list& x, Predicate pred)
    {
        if (this == &x)
            return;
        iterator i = begin();
        iterator j = x.begin();
        while (i != end() && j != x.end())
        {
            if (!pred(*j, *i))
                ++i;
            else
            {
                iterator next = j;
                ++next;
                i = insert(i, *j);
                j = next;
                ++i;
            }
        }
        while (j != x.end())
        {
            i = insert(i, *j);
            ++j;
            ++i;
        }
        x.clear();
    }

    void splice(iterator pos, list& x)
    {
        if (this == &x || x.empty())
            return;
        size_type moved = x._size;
        node_ptr first = x._sentinel->next;
        node_ptr last = x._sentinel->prev;
        node_ptr p = pos.getNode();

        first->prev = p->prev;
        p->prev->next = first;
        last->next = p;
        p->prev = last;

        x._sentinel->next = x._sentinel;
        x._sentinel->prev = x._sentinel;
        x._size = 0;
        _size += moved;
    }

    void splice(iterator pos, list& x, iterator i)
    {
        if (this == &x)
        {
            if (pos == i || ++pos == i)
                return;
            iterator next = i;
            ++next;
            splice(pos, x, i, next);
        }
        else
        {
            iterator next = i;
            ++next;
            splice(pos, x, i, next);
        }
    }

    void splice(iterator pos, list& x, iterator first, iterator last)
    {
        if (this == &x || first == last)
            return;
        size_type count = ft::distance(first, last);
        node_ptr f = first.getNode();
        node_ptr l = last.getNode(); // one past the last moved node
        node_ptr f_prev = f->prev;
        node_ptr l_prev = l->prev;  // last node to move
        node_ptr p = pos.getNode();

        // detach [f, l) from x
        f_prev->next = l;
        l->prev = f_prev;
        // insert before p
        f->prev = p->prev;
        p->prev->next = f;
        l_prev->next = p;
        p->prev = l_prev;

        _size += count;
        x._size -= count;
    }

private:
    allocator_type   _alloc;
    node_allocator_type _node_alloc;
    node_ptr         _sentinel;
    size_type        _size;
};

// ----------------------------------------------------- relational ops
template <class T, class Alloc>
bool operator==(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs)
{
    if (lhs.size() != rhs.size())
        return false;
    typename list<T, Alloc>::const_iterator i = lhs.begin();
    typename list<T, Alloc>::const_iterator j = rhs.begin();
    for (; i != lhs.end(); ++i, ++j)
        if (*i != *j)
            return false;
    return true;
}

template <class T, class Alloc>
bool operator!=(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class Alloc>
bool operator<(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs)
{
    return ft::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T, class Alloc>
bool operator>(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs)
{
    return rhs < lhs;
}

template <class T, class Alloc>
bool operator<=(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs)
{
    return !(rhs < lhs);
}

template <class T, class Alloc>
bool operator>=(const list<T, Alloc>& lhs, const list<T, Alloc>& rhs)
{
    return !(lhs < rhs);
}

template <class T, class Alloc>
void swap(list<T, Alloc>& lhs, list<T, Alloc>& rhs)
{
    lhs.swap(rhs);
}

} // namespace ft

#endif // LIST_HPP
