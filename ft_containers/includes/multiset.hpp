#ifndef MULTISET_HPP
# define MULTISET_HPP

//////////////////////////////////////////////////////////////////////////
//  multiset.hpp — ft::multiset<Key, Compare, Alloc> (C++98, RB-tree)
//  Allows duplicate keys (always inserts).
/////////////////////////////////////////////////////////////////////////

# include <memory>
# include "type_traits.hpp"
# include "iterator.hpp"
# include "utility.hpp"
# include "functional.hpp"
# include "algorithm.hpp"
# include "rb_tree.hpp"

namespace ft
{

template <class Key, class Compare = ft::less<Key>,
          class Alloc = std::allocator<Key> >
class multiset
{
public:
    typedef Key        key_type;
    typedef Key        value_type;
    typedef Compare    key_compare;
    typedef Compare    value_compare;
    typedef Alloc      allocator_type;
    typedef typename Alloc::size_type        size_type;
    typedef typename Alloc::difference_type  difference_type;

    typedef typename rb_tree<value_type, value_compare, Alloc>::iterator       iterator;
    typedef typename rb_tree<value_type, value_compare, Alloc>::const_iterator const_iterator;
    typedef typename rb_tree<value_type, value_compare, Alloc>::reverse_iterator       reverse_iterator;
    typedef typename rb_tree<value_type, value_compare, Alloc>::const_reverse_iterator const_reverse_iterator;

    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef typename Alloc::pointer      pointer;
    typedef typename Alloc::const_pointer const_pointer;

private:
    typedef rb_tree<value_type, value_compare, Alloc> tree_type;
    tree_type _tree;
    key_compare _key_comp;

public:
    explicit multiset(const Compare& comp = Compare(),
                      const allocator_type& alloc = allocator_type())
        : _tree(comp, alloc), _key_comp(comp)
    {
    }

    template <class InputIterator>
    multiset(InputIterator first, InputIterator last,
             const Compare& comp = Compare(),
             const allocator_type& alloc = allocator_type())
        : _tree(comp, alloc), _key_comp(comp)
    {
        while (first != last)
        {
            insert(*first);
            ++first;
        }
    }

    multiset(const multiset& x) : _tree(x._tree), _key_comp(x._key_comp) {}
    ~multiset(void) {}

    multiset& operator=(const multiset& x)
    {
        if (this != &x)
        {
            _tree = x._tree;
            _key_comp = x._key_comp;
        }
        return *this;
    }

    iterator begin(void) { return _tree.begin(); }
    const_iterator begin(void) const { return _tree.begin(); }
    iterator end(void) { return _tree.end(); }
    const_iterator end(void) const { return _tree.end(); }

    key_compare key_comp(void) const { return _key_comp; }

    reverse_iterator rbegin(void) { return _tree.rbegin(); }
    const_reverse_iterator rbegin(void) const { return _tree.rbegin(); }
    reverse_iterator rend(void) { return _tree.rend(); }
    const_reverse_iterator rend(void) const { return _tree.rend(); }

    size_type size(void) const { return _tree.size(); }
    bool      empty(void) const { return _tree.empty(); }
    size_type max_size(void) const { return _tree.max_size(); }

    iterator insert(const value_type& val)
    {
        return _tree.insert_node(val);
    }

    iterator insert(const_iterator hint, const value_type& val)
    {
        (void)hint;
        return insert(val);
    }

    iterator insert(iterator hint, const value_type& val)
    {
        (void)hint;
        return insert(val);
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

    size_type erase(const key_type& key)
    {
        size_type n = 0;
        iterator it = lower_bound(key);
        iterator ub = upper_bound(key);
        while (it != ub)
        {
            it = _tree.erase(it);
            ++n;
        }
        return n;
    }

    iterator erase(iterator pos) { return _tree.erase(pos); }
    iterator erase(iterator first, iterator last)
    {
        while (first != last)
            first = erase(first);
        return first;
    }

    iterator       find(const key_type& key) { return _tree.find(key); }
    const_iterator find(const key_type& key) const { return _tree.find(key); }

    iterator       lower_bound(const key_type& key) { return _tree.lower_bound(key); }
    const_iterator lower_bound(const key_type& key) const { return _tree.lower_bound(key); }

    iterator       upper_bound(const key_type& key) { return _tree.upper_bound(key); }
    const_iterator upper_bound(const key_type& key) const { return _tree.upper_bound(key); }

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
    void swap(multiset& x) { _tree.swap(x._tree); }
};

template <class Key, class Compare, class Alloc>
void swap(multiset<Key, Compare, Alloc>& lhs, multiset<Key, Compare, Alloc>& rhs)
{
    lhs.swap(rhs);
}


// ----------------------------------------------------- relational ops
template <class Key, class Compare, class Alloc>
bool operator==(const multiset<Key, Compare, Alloc>& lhs, const multiset<Key, Compare, Alloc>& rhs)
{
    return lhs.size() == rhs.size() &&
           ft::equal(lhs.begin(), lhs.end(), rhs.begin());
}
template <class Key, class Compare, class Alloc>
bool operator!=(const multiset<Key, Compare, Alloc>& lhs, const multiset<Key, Compare, Alloc>& rhs) { return !(lhs == rhs); }
template <class Key, class Compare, class Alloc>
bool operator<(const multiset<Key, Compare, Alloc>& lhs, const multiset<Key, Compare, Alloc>& rhs)
{
    return ft::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <class Key, class Compare, class Alloc>
bool operator>(const multiset<Key, Compare, Alloc>& lhs, const multiset<Key, Compare, Alloc>& rhs) { return rhs < lhs; }
template <class Key, class Compare, class Alloc>
bool operator<=(const multiset<Key, Compare, Alloc>& lhs, const multiset<Key, Compare, Alloc>& rhs) { return !(rhs < lhs); }
template <class Key, class Compare, class Alloc>
bool operator>=(const multiset<Key, Compare, Alloc>& lhs, const multiset<Key, Compare, Alloc>& rhs) { return !(lhs < rhs); }

} // namespace ft

#endif // MULTISET_HPP
