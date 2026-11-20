#include "AForm.hpp"

const char *AForm::GradeTooHighException::what() const throw()
{
    return "Grade too high!";
}

const char *AForm::GradeTooLowException::what() const throw()
{
    return "Grade too low!";
}

const char *AForm::SignatureNeededException::what() const throw()
{
    return "Form is not signed!";
}

AForm::AForm(const std::string &name, int gradeSign, int gradeExec)
    : _name(name), _signed(false), _gradeSign(gradeSign), _gradeExec(gradeExec)
{
    if (gradeSign < 1)
        throw GradeTooHighException();
    if (gradeSign > 150)
        throw GradeTooLowException();
    if (gradeExec < 1)
        throw GradeTooHighException();
    if (gradeExec > 150)
        throw GradeTooLowException();
}

AForm::AForm(const AForm &src)
    : _name(src._name), _signed(src._signed), _gradeSign(src._gradeSign), _gradeExec(src._gradeExec)
{
}

AForm::~AForm(void)
{
}

AForm &AForm::operator=(const AForm &rhs)
{
    if (this != &rhs)
        _signed = rhs._signed;
    return *this;
}

std::string AForm::getName(void) const
{
    return _name;
}

bool AForm::getSigned(void) const
{
    return _signed;
}

int AForm::getGradeSign(void) const
{
    return _gradeSign;
}

int AForm::getGradeExec(void) const
{
    return _gradeExec;
}

void AForm::beSigned(const Bureaucrat &b)
{
    if (b.getGrade() <= _gradeSign)
        _signed = true;
    else
        throw GradeTooLowException();
}

void AForm::execute(Bureaucrat const &executor) const
{
    if (!_signed)
        throw SignatureNeededException();
    if (executor.getGrade() > _gradeExec)
        throw GradeTooLowException();
    executeAction(executor);
}

std::ostream &operator<<(std::ostream &out, const AForm &f)
{
    out << "Form " << f.getName() << ", sign grade " << f.getGradeSign()
        << ", exec grade " << f.getGradeExec()
        << ", signed: " << (f.getSigned() ? "yes" : "no");
    return out;
}
