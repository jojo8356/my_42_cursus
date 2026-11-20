#include "Intern.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"

Intern::Intern(void)
{
}

Intern::Intern(const Intern &src)
{
    (void)src;
}

Intern::~Intern(void)
{
}

Intern &Intern::operator=(const Intern &rhs)
{
    (void)rhs;
    return *this;
}

AForm *Intern::makeForm(const std::string &name, const std::string &formType) const
{
    if (formType == "shrubbery")
        return new ShrubberyCreationForm(name);
    if (formType == "robotomy request")
        return new RobotomyRequestForm(name);
    if (formType == "presidential pardon")
        return new PresidentialPardonForm(name);
    std::cerr << "Unknown form type: " << formType << std::endl;
    return NULL;
}
