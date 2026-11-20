#include "easyfind.hpp"
#include <iostream>
#include <vector>
#include <list>

int main(void)
{
    std::vector<int> v;
    v.push_back(2);
    v.push_back(42);
    v.push_back(17);

    std::cout << *easyfind(v, 42) << std::endl;
    std::cout << *easyfind(v, 17) << std::endl;

    try
    {
        std::cout << *easyfind(v, 1000) << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    std::list<int> l;
    l.push_back(3);
    l.push_back(9);
    std::cout << *easyfind(l, 9) << std::endl;

    return 0;
}
