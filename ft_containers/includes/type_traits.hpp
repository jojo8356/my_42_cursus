#ifndef TYPE_TRAITS_HPP
# define TYPE_TRAITS_HPP

//////////////////////////////////////////////////////////////////////////
//  type_traits.hpp — enable_if, is_integral, is_same, is_pointer,
//  conditional, iterator tags (C++98, namespace ft)
//////////////////////////////////////////////////////////////////////////

# include <cstddef> // ptrdiff_t
# include <iterator> // std iterator tags (for std::iterator_traits compatibility)

namespace ft
{

// --------------------------- enable_if -------------------------------
template <bool B, typename T = void>
struct enable_if
{
};

template <typename T>
struct enable_if<true, T>
{
    typedef T type;
};

// --------------------------- is_same ---------------------------------
template <typename T, typename U>
struct is_same
{
    static const bool value = false;
};

template <typename T>
struct is_same<T, T>
{
    static const bool value = true;
};

// --------------------------- is_pointer ------------------------------
template <typename T>
struct is_pointer
{
    static const bool value = false;
};

template <typename T>
struct is_pointer<T*>
{
    static const bool value = true;
};

// --------------------------- conditional -----------------------------
template <bool B, typename T, typename F>
struct conditional
{
    typedef T type;
};

template <typename T, typename F>
struct conditional<false, T, F>
{
    typedef F type;
};

// --------------------------- is_integral -----------------------------
template <typename T>
struct is_integral
{
    static const bool value = false;
};

# define FT_IS_INTEGRAL_TYPE(T) \
    template <> struct is_integral<T> { static const bool value = true; }

FT_IS_INTEGRAL_TYPE(bool);
FT_IS_INTEGRAL_TYPE(char);
FT_IS_INTEGRAL_TYPE(signed char);
FT_IS_INTEGRAL_TYPE(unsigned char);
FT_IS_INTEGRAL_TYPE(wchar_t);
FT_IS_INTEGRAL_TYPE(short int);
FT_IS_INTEGRAL_TYPE(unsigned short int);
FT_IS_INTEGRAL_TYPE(int);
FT_IS_INTEGRAL_TYPE(unsigned int);
FT_IS_INTEGRAL_TYPE(long int);
FT_IS_INTEGRAL_TYPE(unsigned long int);
FT_IS_INTEGRAL_TYPE(long long int);
FT_IS_INTEGRAL_TYPE(unsigned long long int);

# undef FT_IS_INTEGRAL_TYPE

// --------------------- is_trivially_copyable -----------------------
// Conservative C++98 approximation: true for arithmetic and pointer
// types (whose copies can be performed with raw memory moves without
// invoking constructors/destructors). Used to pick a memmove-based
// element shift that does not over-count constructions/destructions.
template <typename T>
struct is_trivially_copyable
{
    static const bool value = is_integral<T>::value || is_pointer<T>::value;
};

template <typename T>
struct is_trivially_copyable<const T>
{
    static const bool value = is_trivially_copyable<T>::value;
};

template <typename T>
struct is_trivially_copyable<volatile T>
{
    static const bool value = is_trivially_copyable<T>::value;
};

template <typename T>
struct is_trivially_copyable<const volatile T>
{
    static const bool value = is_trivially_copyable<T>::value;
};

// ------------------- integral_constant -----------------------------
template <bool B>
struct integral_constant
{
    static const bool value = B;
    typedef integral_constant<B> type;
    operator bool() const { return B; }
};

typedef integral_constant<true>   true_type;
typedef integral_constant<false>  false_type;

///////////////////////////////////////////////////////////////////////
//  Iterator tags
///////////////////////////////////////////////////////////////////////
struct input_iterator_tag : public std::input_iterator_tag
{
};
struct output_iterator_tag : public std::output_iterator_tag
{
};
struct forward_iterator_tag : public std::forward_iterator_tag
{
};
struct bidirectional_iterator_tag : public std::bidirectional_iterator_tag
{
};
struct random_access_iterator_tag : public std::random_access_iterator_tag
{
};

typedef std::ptrdiff_t ptrdiff_t;
typedef std::size_t size_t;

} // namespace ft

#endif // TYPE_TRAITS_HPP
