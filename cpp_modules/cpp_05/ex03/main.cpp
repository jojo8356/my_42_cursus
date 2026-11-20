#include "Intern.hpp"
#include "Bureaucrat.hpp"
#include <ctime>
#include <cstdlib>

int main(void)
{
    std::srand(std::time(NULL));
    try
    {
        Intern intern;
        Bureaucrat b("John", 1);

        AForm *f1 = intern.makeForm("home", "shrubbery");
        AForm *f2 = intern.makeForm("robot", "robotomy request");
        AForm *f3 = intern.makeForm("guy", "presidential pardon");

        b.signForm(*f1);
        b.executeForm(*f1);
        b.signForm(*f2);
        b.executeForm(*f2);
        b.signForm(*f3);
        b.executeForm(*f3);

        AForm *f4 = intern.makeForm("bad", "unknown type");
        if (f4 == NULL)
            std::cout << "Unknown form type handled" << std::endl;
        delete f4;

        delete f1;
        delete f2;
        delete f3;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
