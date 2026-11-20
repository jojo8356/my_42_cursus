#include "Base.hpp"
#include "A.hpp"
#include "B.hpp"
#include "C.hpp"
#include <iostream>
#include <cstdlib>
#include <typeinfo>

Base *generate(void)
{
    switch (std::rand() % 3)
    {
        case 0:
            return new A();
        case 1:
            return new B();
        default:
            return new C();
    }
}

void identify(Base *p)
{
    try
    {
        A *a = dynamic_cast<A *>(p);
        if (a)
        {
            std::cout << "A" << std::endl;
            return;
        }
        B *b = dynamic_cast<B *>(p);
        if (b)
        {
            std::cout << "B" << std::endl;
            return;
        }
        C *c = dynamic_cast<C *>(p);
        if (c)
        {
            std::cout << "C" << std::endl;
            return;
        }
    }
    catch (std::bad_cast &)
    {
    }
    std::cout << "Unknown" << std::endl;
}

int main(void)
{
    std::srand(42);
    for (int i = 0; i < 6; ++i)
    {
        Base *p = generate();
        identify(p);
        delete p;
    }
    return 0;
}
