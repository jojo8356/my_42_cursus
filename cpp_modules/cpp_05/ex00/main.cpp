#include "Bureaucrat.hpp"

int main(void)
{
    try
    {
        Bureaucrat b("John", 10);
        std::cout << b << std::endl;
        b.increment();
        std::cout << b << std::endl;
        b.decrement();
        std::cout << b << std::endl;

        Bureaucrat c("Jane", 1);
        c.increment();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    try
    {
        Bureaucrat d("Bad", 151);
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    try
    {
        Bureaucrat e("Low", 0);
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
