#include "iter.hpp"
#include <iostream>
#include <string>

template <typename T>
void print(const T &x)
{
    std::cout << x << std::endl;
}

int main(void)
{
    int tab[] = {0, 1, 2, 42, 4242};
    ::iter(tab, 5, &print<int>);
    std::string tab2[] = {"un", "deux", "trois", "quatre", "cinq"};
    ::iter(tab2, 5, &print<std::string>);
    const int tab3[] = {1, 2, 3};
    ::iter(tab3, 3, &print<int>);
    return 0;
}
