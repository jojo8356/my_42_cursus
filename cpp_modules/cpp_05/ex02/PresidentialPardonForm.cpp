#include "PresidentialPardonForm.hpp"

PresidentialPardonForm::PresidentialPardonForm(const std::string &target)
    : AForm("PresidentialPardonForm", 25, 5), _target(target)
{
}

PresidentialPardonForm::~PresidentialPardonForm(void)
{
}

std::string PresidentialPardonForm::getTarget(void) const
{
    return _target;
}

void PresidentialPardonForm::executeAction(Bureaucrat const &executor) const
{
    (void)executor;
    std::cout << _target << " has been pardoned by Zaphod Beeblebrox." << std::endl;
}
