#include "Point.hpp"
#include <iostream>

int main(void)
{
    Point a(0.0f, 0.0f);
    Point b(4.0f, 0.0f);
    Point c(0.0f, 4.0f);

    std::cout << "point (1,1) inside: " << bsp(a, b, c, Point(1.0f, 1.0f)) << std::endl;
    std::cout << "point (0,0) vertex: " << bsp(a, b, c, Point(0.0f, 0.0f)) << std::endl;
    std::cout << "point (2,2) on edge: " << bsp(a, b, c, Point(2.0f, 2.0f)) << std::endl;
    std::cout << "point (5,5) outside: " << bsp(a, b, c, Point(5.0f, 5.0f)) << std::endl;
    std::cout << "point (1,3) inside: " << bsp(a, b, c, Point(1.0f, 3.0f)) << std::endl;
    return 0;
}
