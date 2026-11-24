#ifndef UTILITY_HPP
# define UTILITY_HPP

//////////////////////////////////////////////////////////////////////////
//  utility.hpp — pair, make_pair, swap (C++98, namespace ft)
//////////////////////////////////////////////////////////////////////////

namespace ft
{

template <class T1, class T2>
struct pair
{
    typedef T1 first_type;
    typedef T2 second_type;

    T1 first;
    T2 second;

    pair(void) : first(), second() {}
    pair(T1 x, T2 y) : first(x), second(y) {}
    template <class U1, class U2>
    pair(U1 x, U2 y) : first(x), second(y) {}
    pair(const pair& x) : first(x.first), second(x.second) {}
    template <class U1, class U2>
    pair(pair<U1, U2> x) : first(x.first), second(x.second) {}

    pair& operator=(const pair& x)
    {
        first = x.first;
        second = x.second;
        return *this;
    }
};

template <class T1, class T2>
pair<T1, T2> make_pair(T1 x, T2 y)
{
    return pair<T1, T2>(x, y);
}

template <class T>
void swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

// --------------------------- pair relational ops -------------------
template <class T1, class T2>
bool operator==(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class T1, class T2>
bool operator!=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return !(lhs == rhs);
}

template <class T1, class T2>
bool operator<(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return lhs.first < rhs.first ||
           (lhs.first == rhs.first && lhs.second < rhs.second);
}

template <class T1, class T2>
bool operator>(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return rhs < lhs;
}

template <class T1, class T2>
bool operator<=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return !(rhs < lhs);
}

template <class T1, class T2>
bool operator>=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
{
    return !(lhs < rhs);
}

} // namespace ft

#endif // UTILITY_HPP
