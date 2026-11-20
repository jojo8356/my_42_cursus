#include "ShrubberyCreationForm.hpp"
#include <fstream>

ShrubberyCreationForm::ShrubberyCreationForm(const std::string &target)
    : AForm("ShrubberyCreationForm", 145, 137), _target(target)
{
}

ShrubberyCreationForm::~ShrubberyCreationForm(void)
{
}

std::string ShrubberyCreationForm::getTarget(void) const
{
    return _target;
}

void ShrubberyCreationForm::executeAction(Bureaucrat const &executor) const
{
    (void)executor;
    std::string filename = _target + "_shrubbery";
    std::ofstream out(filename.c_str());
    if (!out)
        return;
    out << "     *\n";
    out << "    ***\n";
    out << "   *****\n";
    out << "  *******\n";
    out << " ***********\n";
    out << "***************\n";
    out.close();
}
