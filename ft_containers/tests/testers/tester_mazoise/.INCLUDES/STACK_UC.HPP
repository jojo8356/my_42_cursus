#ifndef STACK_HPP
# define STACK_HPP

//////////////////////////////////////////////////////////////////////////
//  stack.hpp — ft::stack<T, Container> (C++98, LIFO adapter)
//  Default underlying container: ft::vector (per subject).
//////////////////////////////////////////////////////////////////////////

# include "vector.hpp"

namespace ft
{

template <class T, class Container = vector<T> >
class stack
{
public:
    typedef T                        value_type;
    typedef Container                container_type;
    typedef typename Container::size_type size_type;

    explicit stack(const container_type& ctnr = container_type())
        : c(ctnr)
    {
    }

    size_type size(void) const { return c.size(); }
    bool      empty(void) const { return c.empty(); }

    value_type&       top(void) { return c.back(); }
    const value_type& top(void) const { return c.back(); }

    void push(const value_type& val) { c.push_back(val); }
    void pop(void) { c.pop_back(); }

protected:
    container_type c;

    friend bool operator==(const stack& lhs, const stack& rhs) { return lhs.c == rhs.c; }
    friend bool operator<(const stack& lhs, const stack& rhs) { return lhs.c < rhs.c; }
};

template <class T, class Container>
bool operator!=(const stack<T, Container>& lhs, const stack<T, Container>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class Container>
bool operator<=(const stack<T, Container>& lhs, const stack<T, Container>& rhs)
{
    return !(rhs < lhs);
}

template <class T, class Container>
bool operator>(const stack<T, Container>& lhs, const stack<T, Container>& rhs)
{
    return rhs < lhs;
}

template <class T, class Container>
bool operator>=(const stack<T, Container>& lhs, const stack<T, Container>& rhs)
{
    return !(lhs < rhs);
}

} // namespace ft

#endif // STACK_HPP
