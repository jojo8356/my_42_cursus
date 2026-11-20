#include "Bureaucrat.hpp"
#include "AForm.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"
#include <ctime>
#include <cstdlib>

int main(void)
{
    std::srand(std::time(NULL));
    try
    {
        Bureaucrat b("John", 1);
        ShrubberyCreationForm shrub("home");
        RobotomyRequestForm robot("robot");
        PresidentialPardonForm pardon("criminal");

        b.executeForm(shrub);
        b.signForm(shrub);
        b.executeForm(shrub);

        b.signForm(robot);
        b.executeForm(robot);

        b.signForm(pardon);
        b.executeForm(pardon);
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
