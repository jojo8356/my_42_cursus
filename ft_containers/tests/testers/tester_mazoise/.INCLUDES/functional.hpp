#ifndef FUNCTIONAL_HPP
# define FUNCTIONAL_HPP

//////////////////////////////////////////////////////////////////////////
//  functional.hpp — binary_function, unary_function, less, greater,
//  equal_to, not_equal_to (C++98, namespace ft)
//////////////////////////////////////////////////////////////////////////

namespace ft
{

template <class Arg1, class Arg2, class Res>
struct binary_function
{
    typedef Arg1 first_argument_type;
    typedef Arg2 second_argument_type;
    typedef Res  result_type;
};

template <class Arg, class Res>
struct unary_function
{
    typedef Arg  argument_type;
    typedef Res  result_type;
};

template <class T>
struct less : public binary_function<T, T, bool>
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs < rhs; }
};

template <class T>
struct greater : public binary_function<T, T, bool>
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs > rhs; }
};

template <class T>
struct equal_to : public binary_function<T, T, bool>
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }
};

template <class T>
struct not_equal_to : public binary_function<T, T, bool>
{
    bool operator()(const T& lhs, const T& rhs) const { return lhs != rhs; }
};

} // namespace ft

#endif // FUNCTIONAL_HPP
