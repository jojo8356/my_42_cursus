/* ************************************************************************** */
/*                                                                          */
/*  priority_queue.hpp — adapteur de priorité (C++98, namespace ft)        */
/*  Bonus : priority_queue sur ft::vector avec opérations de heap          */
/*                                                                          */
/* ************************************************************************** */
#ifndef FT_PRIORITY_QUEUE_HPP
#define FT_PRIORITY_QUEUE_HPP

# include <functional>
# include "vector.hpp"

namespace ft
{

template <class T, class Container = vector<T>, class Compare = std::less<typename Container::value_type> >
class priority_queue
{
public:
    typedef typename Container::value_type        value_type;
    typedef typename Container::size_type         size_type;
    typedef typename Container::reference         reference;
    typedef typename Container::const_reference   const_reference;
    typedef Container                             container_type;
    typedef Compare                               compare_type;

private:
    Container   c;
    Compare     comp;

    // Sift the last element up to restore the heap invariant.
    void _sift_up(size_type idx)
    {
        value_type val = c[idx];
        while (idx > 0)
        {
            size_type parent = (idx - 1) / 2;
            if (comp(c[parent], val))
                break;
            c[idx] = c[parent];
            idx = parent;
        }
        c[idx] = val;
    }

    // Sift the element at idx down to restore the heap invariant.
    void _sift_down(size_type idx)
    {
        value_type val = c[idx];
        size_type n = c.size();
        while (true)
        {
            size_type left = 2 * idx + 1;
            size_type right = 2 * idx + 2;
            size_type best = idx;
            if (left < n && comp(c[best], c[left]))
                best = left;
            if (right < n && comp(c[best], c[right]))
                best = right;
            if (best == idx)
                break;
            c[idx] = c[best];
            idx = best;
        }
        c[idx] = val;
    }

public:
    // Constructors
    priority_queue(const Compare& x = Compare(), const Container& cont = Container())
        : c(cont), comp(x)
    {
        _make_heap();
    }

    template <class InputIterator>
    priority_queue(InputIterator first, InputIterator last,
                   const Compare& x = Compare(), const Container& cont = Container())
        : c(cont), comp(x)
    {
        c.clear();
        for (; first != last; ++first)
            c.push_back(*first);
        _make_heap();
    }

    priority_queue(const priority_queue& x)
        : c(x.c), comp(x.comp)
    {
    }

    priority_queue& operator=(const priority_queue& x)
    {
        if (this == &x)
            return *this;
        c = x.c;
        comp = x.comp;
        return *this;
    }

    ~priority_queue(void)
    {
    }

    bool empty(void) const { return c.empty(); }
    size_type size(void) const { return c.size(); }

    const_reference top(void) const { return c.front(); }
    reference top(void) { return c.front(); }

    void push(const value_type& val)
    {
        c.push_back(val);
        _sift_up(c.size() - 1);
    }

    void pop(void)
    {
        if (c.empty())
            return;
        c[0] = c[c.size() - 1];
        c.pop_back();
        if (!c.empty())
            _sift_down(0);
    }

    void swap(priority_queue& x)
    {
        c.swap(x.c);
        ft::swap(comp, x.comp);
    }

    const Container& container(void) const { return c; }
    Container& container(void) { return c; }
    const Compare& get_compare(void) const { return comp; }

private:
    void _make_heap(void)
    {
        // Bottom-up heapify.
        for (size_type i = (c.size() / 2); i-- > 0;)
            _sift_down(i);
    }
};

template <class T, class Container, class Compare>
void swap(priority_queue<T, Container, Compare>& lhs,
          priority_queue<T, Container, Compare>& rhs)
{
    lhs.swap(rhs);
}

} // namespace ft

#endif // FT_PRIORITY_QUEUE_HPP
