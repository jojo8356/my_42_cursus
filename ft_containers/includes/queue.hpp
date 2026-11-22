#ifndef QUEUE_HPP
# define QUEUE_HPP

//////////////////////////////////////////////////////////////////////////
//  queue.hpp — ft::queue<T, Container> (C++98, FIFO adapter)
//  Default underlying container: ft::deque (per std::queue).
//////////////////////////////////////////////////////////////////////////

# include "deque.hpp"

namespace ft
{

template <class T, class Container = deque<T> >
class queue
{
public:
    typedef T                        value_type;
    typedef Container                container_type;
    typedef typename Container::size_type size_type;

    explicit queue(const container_type& ctnr = container_type())
        : c(ctnr)
    {
    }

    size_type size(void) const { return c.size(); }
    bool      empty(void) const { return c.empty(); }

    value_type&       front(void) { return c.front(); }
    const value_type& front(void) const { return c.front(); }
    value_type&       back(void) { return c.back(); }
    const value_type& back(void) const { return c.back(); }

    void push(const value_type& val) { c.push_back(val); }
    void pop(void) { c.pop_front(); }

protected:
    container_type c;

    friend bool operator==(const queue& lhs, const queue& rhs) { return lhs.c == rhs.c; }
    friend bool operator<(const queue& lhs, const queue& rhs) { return lhs.c < rhs.c; }
};

template <class T, class Container>
bool operator!=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class Container>
bool operator<=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return !(rhs < lhs);
}

template <class T, class Container>
bool operator>(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return rhs < lhs;
}

template <class T, class Container>
bool operator>=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
    return !(lhs < rhs);
}

} // namespace ft

#endif // QUEUE_HPP
