#include "Span.hpp"
#include <iostream>
#include <vector>

int main(void)
{
    Span sp = Span(5);
    sp.addNumber(6);
    sp.addNumber(3);
    sp.addNumber(17);
    sp.addNumber(9);
    sp.addNumber(11);
    std::cout << sp.shortestSpan() << std::endl;
    std::cout << sp.longestSpan() << std::endl;

    std::vector<int> big;
    for (int i = 0; i < 10000; ++i)
        big.push_back(i * 2);
    Span sp2 = Span(10000);
    sp2.addNumber(big.begin(), big.end());
    std::cout << sp2.shortestSpan() << std::endl;
    std::cout << sp2.longestSpan() << std::endl;

    try
    {
        Span sp3 = Span(1);
        sp3.addNumber(5);
        sp3.addNumber(7);
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
