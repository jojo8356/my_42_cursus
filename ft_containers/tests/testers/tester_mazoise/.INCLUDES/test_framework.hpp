//////////////////////////////////////////////////////////////////////////
//  test_framework.hpp — minimal assertion framework for ft_containers
//////////////////////////////////////////////////////////////////////////

# ifndef TEST_FRAMEWORK_HPP
# define TEST_FRAMEWORK_HPP

# include <iostream>
# include <string>
# include <sstream>

namespace ftctest
{

static int g_pass = 0;
static int g_fail = 0;
static std::string g_current_suite;

inline void suite(const std::string& name)
{
    g_current_suite = name;
    std::cout << "\n=== " << name << " ===" << std::endl;
}

inline void check(bool cond, const std::string& msg)
{
    if (cond)
    {
        ++g_pass;
    }
    else
    {
        ++g_fail;
        std::cout << "  [FAIL] " << g_current_suite << ": " << msg << std::endl;
    }
}

template <class T1, class T2>
void check_eq(const T1& a, const T2& b, const std::string& msg)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
    if (a == b)
#pragma GCC diagnostic pop
        ++g_pass;
    else
    {
        ++g_fail;
        std::ostringstream oss;
        oss << g_current_suite << ": " << msg << " (got " << a << ", expected " << b << ")";
        std::cout << "  [FAIL] " << oss.str() << std::endl;
    }
}

inline int summary()
{
    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Passed: " << g_pass << "   Failed: " << g_fail << std::endl;
    if (g_fail == 0)
        std::cout << "ALL TESTS PASSED" << std::endl;
    else
        std::cout << "SOME TESTS FAILED" << std::endl;
    return g_fail == 0 ? 0 : 1;
}

} // namespace ftctest

# define TEST(name) ftctest::suite(name)
# define ASSERT(cond, msg) ftctest::check((cond), (msg))
# define ASSERT_EQ(a, b, msg) ftctest::check_eq((a), (b), (msg))

# endif // TEST_FRAMEWORK_HPP
