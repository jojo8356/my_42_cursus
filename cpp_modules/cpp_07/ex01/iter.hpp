#ifndef ITER_HPP
#define ITER_HPP

template <typename T, typename Operator>
void iter(T *address, const unsigned len, Operator f)
{
    for (unsigned i = 0; i < len; ++i)
        f(address[i]);
}

template <typename T, typename Operator>
void iter(T const *address, const unsigned len, Operator f)
{
    for (unsigned i = 0; i < len; ++i)
        f(address[i]);
}

#endif
