#include "Bureaucrat.hpp"
#include "Form.hpp"

int main(void)
{
    try
    {
        Bureaucrat b("John", 10);
        Form f("MyForm", 15, 10);
        b.signForm(f);

        Bureaucrat c("Jane", 100);
        Form g("HardForm", 10, 5);
        c.signForm(g);
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
