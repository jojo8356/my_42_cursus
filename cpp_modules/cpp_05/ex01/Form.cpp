#include "Form.hpp"

const char *Form::GradeTooHighException::what() const throw()
{
    return "Grade too high!";
}

const char *Form::GradeTooLowException::what() const throw()
{
    return "Grade too low!";
}

Form::Form(void) : _name("Form"), _signed(false), _gradeSign(150), _gradeExec(150)
{
}

Form::Form(const std::string &name, int gradeSign, int gradeExec)
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

Form::Form(const Form &src)
    : _name(src._name), _signed(src._signed), _gradeSign(src._gradeSign), _gradeExec(src._gradeExec)
{
}

Form::~Form(void)
{
}

Form &Form::operator=(const Form &rhs)
{
    if (this != &rhs)
        _signed = rhs._signed;
    return *this;
}

std::string Form::getName(void) const
{
    return _name;
}

bool Form::getSigned(void) const
{
    return _signed;
}

int Form::getGradeSign(void) const
{
    return _gradeSign;
}

int Form::getGradeExec(void) const
{
    return _gradeExec;
}

void Form::beSigned(const Bureaucrat &b)
{
    if (b.getGrade() <= _gradeSign)
        _signed = true;
    else
        throw GradeTooLowException();
}

std::ostream &operator<<(std::ostream &out, const Form &f)
{
    out << "Form " << f.getName() << ", sign grade " << f.getGradeSign()
        << ", exec grade " << f.getGradeExec()
        << ", signed: " << (f.getSigned() ? "yes" : "no");
    return out;
}
