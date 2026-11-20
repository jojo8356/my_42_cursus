#include "RobotomyRequestForm.hpp"
#include <cstdlib>
#include <ctime>

RobotomyRequestForm::RobotomyRequestForm(const std::string &target)
    : AForm("RobotomyRequestForm", 72, 45), _target(target)
{
}

RobotomyRequestForm::~RobotomyRequestForm(void)
{
}

std::string RobotomyRequestForm::getTarget(void) const
{
    return _target;
}

void RobotomyRequestForm::executeAction(Bureaucrat const &executor) const
{
    (void)executor;
    std::cout << "* Inserting a screwdriver in the ear of " << _target << " *" << std::endl;
    std::cout << "* Bzzzzzz... *" << std::endl;
    if (std::rand() % 2 == 0)
        std::cout << _target << " has been robotomized successfully." << std::endl;
    else
        std::cout << "Robotomy failed on " << _target << "." << std::endl;
}
