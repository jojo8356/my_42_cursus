#ifndef RB_TREE_HPP
# define RB_TREE_HPP

//////////////////////////////////////////////////////////////////////////
//  rb_tree.hpp — ft::rb_tree<V, Compare, Alloc> (C++98, red-black tree)
//  Stores V (pair<const Key,T> for map/multimap, Key for set/multiset).
//  Compare operates on V. insert_node always inserts (wrapper decides
//  unique vs multiple via lower_bound).
//////////////////////////////////////////////////////////////////////////

# include <memory>
# include "type_traits.hpp"
# include "iterator.hpp"
# include "utility.hpp"

namespace ft
{

template <class V, class Compare, class Alloc = std::allocator<V> >
class rb_tree
{
public:
    typedef V                                     value_type;
    typedef Compare                               value_compare;
    typedef Alloc                                 allocator_type;
    typedef typename Alloc::size_type            size_type;
    typedef typename Alloc::difference_type      difference_type;
    typedef typename Alloc::pointer              pointer;
    typedef typename Alloc::const_pointer        const_pointer;
    typedef typename Alloc::reference            reference;
    typedef typename Alloc::const_reference      const_reference;

private:
    struct node
    {
        V       value;
        node*   parent;
        node*   left;
        node*   right;
        bool    red;
        node(const V& v) : value(v), parent(0), left(0), right(0), red(true) {}
    };

    typedef typename Alloc::template rebind<node>::other node_allocator_type;
    typedef typename node_allocator_type::pointer node_ptr;

public:
    // -------------------------------------------------------- Iterator
    class iterator
    {
    public:
        typedef V                                 value_type;
        typedef V&                                reference;
        typedef V*                                pointer;
        typedef ft::ptrdiff_t                     difference_type;
        typedef ft::size_t                       size_type;
        typedef ft::bidirectional_iterator_tag   iterator_category;

        iterator(void) : _tree(0), _node(0) {}
        iterator(const iterator& x) : _tree(x._tree), _node(x._node) {}
        ~iterator(void) {}

        iterator& operator=(const iterator& x)
        {
            _tree = x._tree; _node = x._node;
            return *this;
        }

        bool operator==(const iterator& x) const { return _node == x._node; }
        bool operator!=(const iterator& x) const { return _node != x._node; }

        iterator& operator++(void)
        {
            _node = _tree->successor(_node);
            return *this;
        }
        iterator& operator--(void)
        {
            _node = _tree->predecessor(_node);
            return *this;
        }
        iterator  operator++(int) { iterator x(*this); ++*this; return x; }
        iterator  operator--(int) { iterator x(*this); --*this; return x; }

        reference operator*(void) const { return _node->value; }
        pointer   operator->(void) const { return &(_node->value); }

    private:
        iterator(const rb_tree* tree, node_ptr node) : _tree(tree), _node(node) {}

        const rb_tree* _tree;
        node_ptr       _node;

        friend class rb_tree;
        friend class reverse_iterator<iterator>;
    };

    class const_iterator
    {
    public:
        typedef V                                 value_type;
        typedef const V&                          reference;
        typedef const V*                          pointer;
        typedef ft::ptrdiff_t                     difference_type;
        typedef ft::size_t                       size_type;
        typedef ft::bidirectional_iterator_tag   iterator_category;

        const_iterator(void) : _tree(0), _node(0) {}
        const_iterator(const const_iterator& x) : _tree(x._tree), _node(x._node) {}
        ~const_iterator(void) {}

        const_iterator(const iterator& x) : _tree(x._tree), _node(x._node) {}

        const_iterator& operator=(const const_iterator& x)
        {
            _tree = x._tree; _node = x._node;
            return *this;
        }

        bool operator==(const const_iterator& x) const { return _node == x._node; }
        bool operator!=(const const_iterator& x) const { return _node != x._node; }

        const_iterator& operator++(void)
        {
            _node = _tree->successor(_node);
            return *this;
        }
        const_iterator& operator--(void)
        {
            _node = _tree->predecessor(_node);
            return *this;
        }
        const_iterator  operator++(int) { const_iterator x(*this); ++*this; return x; }
        const_iterator  operator--(int) { const_iterator x(*this); --*this; return x; }

        reference operator*(void) const { return _node->value; }
        pointer   operator->(void) const { return &(_node->value); }

    private:
        const_iterator(const rb_tree* tree, node_ptr node) : _tree(tree), _node(node) {}

        const rb_tree* _tree;
        node_ptr       _node;

        friend class rb_tree;
        friend class reverse_iterator<const_iterator>;
    };

    typedef ft::reverse_iterator<iterator>       reverse_iterator;
    typedef ft::reverse_iterator<const_iterator> const_reverse_iterator;

    // ------------------------------------------------------- Constructors
    explicit rb_tree(const Compare& comp = Compare(),
                     const allocator_type& alloc = allocator_type())
        : _comp(comp), _node_alloc(alloc), _root(0), _size(0)
    {
        _nil = _node_alloc.allocate(1);
        _node_alloc.construct(_nil, node(V()));
        _nil->red = false;
        _nil->parent = _nil;
        _nil->left = _nil;
        _nil->right = _nil;
        _root = _nil;
    }

    ~rb_tree(void)
    {
        clear();
        _node_alloc.destroy(_nil);
        _node_alloc.deallocate(_nil, 1);
    }

    // Copy constructor (deep copy)
    rb_tree(const rb_tree& x)
        : _comp(x._comp), _node_alloc(x._node_alloc), _root(0), _size(0)
    {
        _nil = _node_alloc.allocate(1);
        _node_alloc.construct(_nil, node(V()));
        _nil->red = false;
        _nil->parent = _nil;
        _nil->left = _nil;
        _nil->right = _nil;
        _root = _nil;
        if (x._root != x._nil)
            _root = _copy_subtree(x._root, x._nil, _nil);
        _size = x._size;
    }

    // Assignment operator (deep copy)
    rb_tree& operator=(const rb_tree& x)
    {
        if (this == &x)
            return *this;
        clear();
        _comp = x._comp;
        if (x._root != x._nil)
            _root = _copy_subtree(x._root, x._nil, _nil);
        _size = x._size;
        return *this;
    }

    // ------------------------------------------------------- Iterators
    iterator begin(void) { return iterator(this, leftmost(_root)); }
    const_iterator begin(void) const { return const_iterator(this, leftmost(_root)); }
    iterator end(void) { return iterator(this, _nil); }
    const_iterator end(void) const { return const_iterator(this, _nil); }

    reverse_iterator rbegin(void) { return reverse_iterator(end()); }
    const_reverse_iterator rbegin(void) const { return const_reverse_iterator(end()); }
    reverse_iterator rend(void) { return reverse_iterator(begin()); }
    const_reverse_iterator rend(void) const { return const_reverse_iterator(begin()); }

    // ------------------------------------------------------- Capacity
    size_type size(void) const { return _size; }
    bool      empty(void) const { return _size == 0; }
    size_type max_size(void) const { return allocator_type().max_size(); }

    // expose the comparator (used by wrappers to check key equality)
    const value_compare& comp(void) const { return _comp; }

    // ------------------------------------------------------- Search
    iterator lower_bound(const V& val)
    {
        node_ptr x = _root;
        node_ptr y = _nil;
        while (x != _nil)
        {
            if (_comp(x->value, val))
                x = x->right;
            else
            {
                y = x;
                x = x->left;
            }
        }
        return iterator(this, y);
    }

    const_iterator lower_bound(const V& val) const
    {
        node_ptr x = _root;
        node_ptr y = _nil;
        while (x != _nil)
        {
            if (_comp(x->value, val))
                x = x->right;
            else
            {
                y = x;
                x = x->left;
            }
        }
        return const_iterator(this, y);
    }

    iterator upper_bound(const V& val)
    {
        node_ptr x = _root;
        node_ptr y = _nil;
        while (x != _nil)
        {
            if (_comp(val, x->value))
            {
                y = x;
                x = x->left;
            }
            else
                x = x->right;
        }
        return iterator(this, y);
    }

    const_iterator upper_bound(const V& val) const
    {
        node_ptr x = _root;
        node_ptr y = _nil;
        while (x != _nil)
        {
            if (_comp(val, x->value))
            {
                y = x;
                x = x->left;
            }
            else
                x = x->right;
        }
        return const_iterator(this, y);
    }

    iterator find(const V& val)
    {
        node_ptr x = _root;
        while (x != _nil && (_comp(val, x->value) || _comp(x->value, val)))
        {
            if (_comp(val, x->value))
                x = x->left;
            else
                x = x->right;
        }
        if (x == _nil)
            return end();
        return iterator(this, x);
    }

    const_iterator find(const V& val) const
    {
        node_ptr x = _root;
        while (x != _nil && (_comp(val, x->value) || _comp(x->value, val)))
        {
            if (_comp(val, x->value))
                x = x->left;
            else
                x = x->right;
        }
        if (x == _nil)
            return end();
        return const_iterator(this, x);
    }

    size_type count(const V& val)
    {
        iterator lb = lower_bound(val);
        iterator ub = upper_bound(val);
        size_type n = 0;
        while (lb != ub)
        {
            ++n;
            ++lb;
        }
        return n;
    }

    size_type count(const V& val) const
    {
        const_iterator lb = lower_bound(val);
        const_iterator ub = upper_bound(val);
        size_type n = 0;
        while (lb != ub)
        {
            ++n;
            ++lb;
        }
        return n;
    }

    ft::pair<iterator, iterator> equal_range(const V& val)
    {
        return ft::make_pair(lower_bound(val), upper_bound(val));
    }

    ft::pair<const_iterator, const_iterator> equal_range(const V& val) const
    {
        return ft::make_pair(lower_bound(val), upper_bound(val));
    }

    // ------------------------------------------------------- Insert
    iterator insert_node(const V& val)
    {
        node_ptr z = _node_alloc.allocate(1);
        _node_alloc.construct(z, node(val));
        z->red = true;
        z->parent = _nil;
        z->left = _nil;
        z->right = _nil;

        node_ptr y = _nil;
        node_ptr x = _root;
        while (x != _nil)
        {
            y = x;
            if (_comp(z->value, x->value))
                x = x->left;
            else
                x = x->right;
        }
        z->parent = y;
        if (y == _nil)
            _root = z;
        else if (_comp(z->value, y->value))
            y->left = z;
        else
            y->right = z;

        insert_fixup(z);
        ++_size;
        return iterator(this, z);
    }

    // ------------------------------------------------------- Erase
    iterator erase(iterator pos)
    {
        node_ptr z = pos._node;
        node_ptr succ = successor(z); // element following the erased one
        node_ptr y = z;
        bool y_original_color = y->red;
        node_ptr x;

        if (z->left == _nil)
        {
            x = z->right;
            transplant(z, z->right);
        }
        else if (z->right == _nil)
        {
            x = z->left;
            transplant(z, z->left);
        }
        else
        {
            y = tree_minimum(z->right);
            y_original_color = y->red;
            x = y->right;
            if (y->parent == z)
            {
                if (x != _nil)
                    x->parent = y;
            }
            else
            {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->red = z->red;
        }

        if (y_original_color == false)
            erase_fixup(x);

        _node_alloc.destroy(z);
        _node_alloc.deallocate(z, 1);
        --_size;
        return iterator(this, succ);
    }

    void clear(void)
    {
        erase_subtree(_root);
        _root = _nil;
        _size = 0;
    }

    void swap(rb_tree& x)
    {
        if (this == &x)
            return;
        ft::swap(_root, x._root);
        ft::swap(_size, x._size);
        ft::swap(_nil, x._nil);
        // _comp is not swapped (both trees share the same comparator type)
    }

    Compare  _comp;
    node_allocator_type _node_alloc;
    node_ptr _nil;
    node_ptr _root;
    size_type _size;

    // ----------------------------------------------------- rotations
    void left_rotate(node_ptr x)
    {
        node_ptr y = x->right;
        x->right = y->left;
        if (y->left != _nil)
            y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == _nil)
            _root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void right_rotate(node_ptr x)
    {
        node_ptr y = x->left;
        x->left = y->right;
        if (y->right != _nil)
            y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == _nil)
            _root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->right = x;
        x->parent = y;
    }

    // ----------------------------------------------------- fixups
    void insert_fixup(node_ptr z)
    {
        while (z->parent->red)
        {
            if (z->parent == z->parent->parent->left)
            {
                node_ptr y = z->parent->parent->right;
                if (y->red)
                {
                    z->parent->red = false;
                    y->red = false;
                    z->parent->parent->red = true;
                    z = z->parent->parent;
                }
                else
                {
                    if (z == z->parent->right)
                    {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->red = false;
                    z->parent->parent->red = true;
                    right_rotate(z->parent->parent);
                }
            }
            else
            {
                node_ptr y = z->parent->parent->left;
                if (y->red)
                {
                    z->parent->red = false;
                    y->red = false;
                    z->parent->parent->red = true;
                    z = z->parent->parent;
                }
                else
                {
                    if (z == z->parent->left)
                    {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->red = false;
                    z->parent->parent->red = true;
                    left_rotate(z->parent->parent);
                }
            }
        }
        _root->red = false;
    }

    void erase_fixup(node_ptr x)
    {
        while (x != _root && x != _nil && !x->red)
        {
            if (x == x->parent->left)
            {
                node_ptr w = x->parent->right;
                if (w->red)
                {
                    w->red = false;
                    x->parent->red = true;
                    left_rotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->red == false && w->right->red == false)
                {
                    w->red = true;
                    x = x->parent;
                }
                else
                {
                    if (w->right->red == false)
                    {
                        w->left->red = false;
                        w->red = true;
                        right_rotate(w);
                        w = x->parent->right;
                    }
                    w->red = x->parent->red;
                    x->parent->red = false;
                    w->right->red = false;
                    left_rotate(x->parent);
                    x = _root;
                }
            }
            else
            {
                node_ptr w = x->parent->left;
                if (w->red)
                {
                    w->red = false;
                    x->parent->red = true;
                    right_rotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->red == false && w->left->red == false)
                {
                    w->red = true;
                    x = x->parent;
                }
                else
                {
                    if (w->left->red == false)
                    {
                        w->right->red = false;
                        w->red = true;
                        left_rotate(w);
                        w = x->parent->left;
                    }
                    w->red = x->parent->red;
                    x->parent->red = false;
                    w->left->red = false;
                    right_rotate(x->parent);
                    x = _root;
                }
            }
        }
        x->red = false;
    }

    // ----------------------------------------------------- helpers
    void transplant(node_ptr u, node_ptr v)
    {
        if (u->parent == _nil)
            _root = v;
        else if (u == u->parent->left)
            u->parent->left = v;
        else
            u->parent->right = v;
        v->parent = u->parent;
    }

    node_ptr tree_minimum(node_ptr x)
    {
        while (x->left != _nil)
            x = x->left;
        return x;
    }

    node_ptr leftmost(node_ptr x) const
    {
        if (x == _nil)
            return _nil;
        while (x->left != _nil)
            x = x->left;
        return x;
    }

    node_ptr successor(node_ptr n) const
    {
        if (n->right != _nil)
        {
            n = n->right;
            while (n->left != _nil)
                n = n->left;
            return n;
        }
        node_ptr p = n->parent;
        while (p != _nil && n == p->right)
        {
            n = p;
            p = p->parent;
        }
        return p;
    }

    node_ptr predecessor(node_ptr n) const
    {
        if (n == _nil)
        {
            n = _root;
            while (n->right != _nil)
                n = n->right;
            return n;
        }
        if (n->left != _nil)
        {
            n = n->left;
            while (n->right != _nil)
                n = n->right;
            return n;
        }
        node_ptr p = n->parent;
        while (p != _nil && n == p->left)
        {
            n = p;
            p = p->parent;
        }
        return p;
    }

    node_ptr _copy_subtree(node_ptr src, node_ptr src_nil, node_ptr parent)
    {
        node_ptr n = _node_alloc.allocate(1);
        _node_alloc.construct(n, node(src->value));
        n->red = src->red;
        n->parent = parent;
        n->left = (src->left == src_nil) ? _nil : _copy_subtree(src->left, src_nil, n);
        n->right = (src->right == src_nil) ? _nil : _copy_subtree(src->right, src_nil, n);
        return n;
    }

    void erase_subtree(node_ptr x)
    {
        if (x == _nil)
            return;
        erase_subtree(x->left);
        erase_subtree(x->right);
        _node_alloc.destroy(x);
        _node_alloc.deallocate(x, 1);
    }
};

} // namespace ft

#endif // RB_TREE_HPP
