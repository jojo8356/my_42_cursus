#ifndef MAP_HPP
# define MAP_HPP

//////////////////////////////////////////////////////////////////////////
//  map.hpp — ft::map<Key, T, Compare, Alloc> (C++98, RB-tree wrapper)
//////////////////////////////////////////////////////////////////////////

# include <memory>
# include <stdexcept>
# include "type_traits.hpp"
# include "iterator.hpp"
# include "utility.hpp"
# include "functional.hpp"
# include "algorithm.hpp"
# include "rb_tree.hpp"

namespace ft
{

template <class Key, class T, class Compare = ft::less<Key>,
          class Alloc = std::allocator<ft::pair<const Key, T> > >
class map
{
public:
    typedef Key                                     key_type;
    typedef T                                       mapped_type;
    typedef ft::pair<const Key, T>                 value_type;
    typedef Compare                                 key_compare;
    typedef Alloc                                   allocator_type;
    typedef typename Alloc::size_type              size_type;
    typedef typename Alloc::difference_type        difference_type;

    // value_compare compares values by key
    class value_compare : public ft::binary_function<value_type, value_type, bool>
    {
        friend class map;
        Compare _comp;
        value_compare(const Compare& comp) : _comp(comp) {}
    public:
        bool operator()(const value_type& a, const value_type& b) const
        {
            return _comp(a.first, b.first);
        }
    };

    typedef typename rb_tree<value_type, value_compare, Alloc>::iterator       iterator;
    typedef typename rb_tree<value_type, value_compare, Alloc>::const_iterator const_iterator;
    typedef typename rb_tree<value_type, value_compare, Alloc>::reverse_iterator       reverse_iterator;
    typedef typename rb_tree<value_type, value_compare, Alloc>::const_reverse_iterator const_reverse_iterator;

    typedef value_type&       reference;
    typedef const value_type& const_reference;

    typedef typename Alloc::pointer           pointer;
    typedef typename Alloc::const_pointer     const_pointer;

private:
    typedef rb_tree<value_type, value_compare, Alloc> tree_type;
    tree_type _tree;
    key_compare _key_comp;

public:
    explicit map(const Compare& comp = Compare(),
                 const allocator_type& alloc = allocator_type())
        : _tree(value_compare(comp), alloc), _key_comp(comp)
    {
    }

    template <class InputIterator>
    map(InputIterator first, InputIterator last,
        const Compare& comp = Compare(),
        const allocator_type& alloc = allocator_type())
        : _tree(value_compare(comp), alloc), _key_comp(comp)
    {
        while (first != last)
        {
            insert(*first);
            ++first;
        }
    }

    map(const map& x) : _tree(x._tree), _key_comp(x._key_comp) {}

    ~map(void) {}

    map& operator=(const map& x)
    {
        if (this != &x)
        {
            _tree = x._tree;
            _key_comp = x._key_comp;
        }
        return *this;
    }

    // ------------------------------------------------------- Iterators
    iterator begin(void) { return _tree.begin(); }
    const_iterator begin(void) const { return _tree.begin(); }
    iterator end(void) { return _tree.end(); }
    const_iterator end(void) const { return _tree.end(); }

    key_compare key_comp(void) const { return _key_comp; }
    value_compare value_comp(void) const { return value_compare(_key_comp); }

    reverse_iterator rbegin(void) { return _tree.rbegin(); }
    const_reverse_iterator rbegin(void) const { return _tree.rbegin(); }
    reverse_iterator rend(void) { return _tree.rend(); }
    const_reverse_iterator rend(void) const { return _tree.rend(); }

    // ------------------------------------------------------- Capacity
    size_type size(void) const { return _tree.size(); }
    bool      empty(void) const { return _tree.empty(); }
    size_type max_size(void) const { return _tree.max_size(); }

    // ------------------------------------------------------- Element access
    mapped_type& operator[](const key_type& key)
    {
        iterator it = lower_bound(key);
        if (it != end() && !_key_comp(key, it->first) && !_key_comp(it->first, key))
            return it->second;
        // not found: insert
        it = insert(ft::make_pair(key, mapped_type())).first;
        return it->second;
    }

    // ------------------------------------------------------- Insert
    ft::pair<iterator, bool> insert(const value_type& val)
    {
        iterator it = lower_bound_value(val);
        if (it != end() && !_key_comp(val.first, it->first) && !_key_comp(it->first, val.first))
            return ft::make_pair(it, false);
        return ft::make_pair(_tree.insert_node(val), true);
    }

    iterator insert(const_iterator hint, const value_type& val)
    {
        (void)hint;
        return insert(val).first;
    }

    iterator insert(iterator hint, const value_type& val)
    {
        (void)hint;
        return insert(val).first;
    }

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        while (first != last)
        {
            insert(*first);
            ++first;
        }
    }

    // ------------------------------------------------------- Erase
    size_type erase(const key_type& key)
    {
        iterator it = find_value(key);
        if (it == end())
            return 0;
        _tree.erase(it);
        return 1;
    }

    void erase(iterator pos) { _tree.erase(pos); }
    void erase(iterator first, iterator last)
    {
        while (first != last)
            first = _tree.erase(first);
    }

    // ------------------------------------------------------- Search
    iterator       find(const key_type& key) { return find_value(key); }
    const_iterator find(const key_type& key) const { return find_value(key); }

    iterator       lower_bound(const key_type& key) { return lower_bound_value(value_type(key, mapped_type())); }
    const_iterator lower_bound(const key_type& key) const { return lower_bound_value(value_type(key, mapped_type())); }

    iterator       upper_bound(const key_type& key) { return upper_bound_value(value_type(key, mapped_type())); }
    const_iterator upper_bound(const key_type& key) const { return upper_bound_value(value_type(key, mapped_type())); }

    ft::pair<iterator, iterator>       equal_range(const key_type& key) { return ft::make_pair(lower_bound(key), upper_bound(key)); }
    ft::pair<const_iterator, const_iterator> equal_range(const key_type& key) const { return ft::make_pair(lower_bound(key), upper_bound(key)); }

    size_type count(const key_type& key) const
    {
        const_iterator lb = lower_bound(key);
        const_iterator ub = upper_bound(key);
        size_type n = 0;
        while (lb != ub)
        {
            ++n;
            ++lb;
        }
        return n;
    }

    void clear(void) { _tree.clear(); }
    void swap(map& x) { _tree.swap(x._tree); ft::swap(_key_comp, x._key_comp); }

private:
    // Build a value_type with a default-constructed mapped_type for searching
    iterator       lower_bound_value(const value_type& val) { return _tree.lower_bound(val); }
    const_iterator lower_bound_value(const value_type& val) const { return _tree.lower_bound(val); }
    iterator       upper_bound_value(const value_type& val) { return _tree.upper_bound(val); }
    const_iterator upper_bound_value(const value_type& val) const { return _tree.upper_bound(val); }
    iterator       find_value(const key_type& key) { return _tree.find(value_type(key, mapped_type())); }
    const_iterator find_value(const key_type& key) const { return _tree.find(value_type(key, mapped_type())); }
};

template <class Key, class T, class Compare, class Alloc>
void swap(map<Key, T, Compare, Alloc>& lhs, map<Key, T, Compare, Alloc>& rhs)
{
    lhs.swap(rhs);
}


// ----------------------------------------------------- relational ops
template <class Key, class T, class Compare, class Alloc>
bool operator==(const map<Key, T, Compare, Alloc>& lhs, const map<Key, T, Compare, Alloc>& rhs)
{
    return lhs.size() == rhs.size() &&
           ft::equal(lhs.begin(), lhs.end(), rhs.begin());
}
template <class Key, class T, class Compare, class Alloc>
bool operator!=(const map<Key, T, Compare, Alloc>& lhs, const map<Key, T, Compare, Alloc>& rhs) { return !(lhs == rhs); }
template <class Key, class T, class Compare, class Alloc>
bool operator<(const map<Key, T, Compare, Alloc>& lhs, const map<Key, T, Compare, Alloc>& rhs)
{
    return ft::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <class Key, class T, class Compare, class Alloc>
bool operator>(const map<Key, T, Compare, Alloc>& lhs, const map<Key, T, Compare, Alloc>& rhs) { return rhs < lhs; }
template <class Key, class T, class Compare, class Alloc>
bool operator<=(const map<Key, T, Compare, Alloc>& lhs, const map<Key, T, Compare, Alloc>& rhs) { return !(rhs < lhs); }
template <class Key, class T, class Compare, class Alloc>
bool operator>=(const map<Key, T, Compare, Alloc>& lhs, const map<Key, T, Compare, Alloc>& rhs) { return !(lhs < rhs); }

} // namespace ft

#endif // MAP_HPP
