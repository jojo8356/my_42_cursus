#include "Bureaucrat.hpp"
#include "AForm.hpp"

const char *Bureaucrat::GradeTooHighException::what() const throw()
{
    return "Grade too high!";
}

const char *Bureaucrat::GradeTooLowException::what() const throw()
{
    return "Grade too low!";
}

Bureaucrat::Bureaucrat(void) : _name("Bureaucrat"), _grade(150)
{
}

Bureaucrat::Bureaucrat(const std::string &name, int grade) : _name(name), _grade(grade)
{
    if (grade < 1)
        throw GradeTooHighException();
    if (grade > 150)
        throw GradeTooLowException();
}

Bureaucrat::Bureaucrat(const Bureaucrat &src) : _name(src._name), _grade(src._grade)
{
}

Bureaucrat::~Bureaucrat(void)
{
}

Bureaucrat &Bureaucrat::operator=(const Bureaucrat &rhs)
{
    if (this != &rhs)
        _grade = rhs._grade;
    return *this;
}

std::string Bureaucrat::getName(void) const
{
    return _name;
}

int Bureaucrat::getGrade(void) const
{
    return _grade;
}

void Bureaucrat::increment(void)
{
    if (_grade <= 1)
        throw GradeTooHighException();
    --_grade;
}

void Bureaucrat::decrement(void)
{
    if (_grade >= 150)
        throw GradeTooLowException();
    ++_grade;
}

void Bureaucrat::signForm(AForm &form)
{
    try
    {
        form.beSigned(*this);
        std::cout << _name << " signed " << form << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << _name << " couldn't sign " << form << " because " << e.what() << "." << std::endl;
    }
}

void Bureaucrat::executeForm(AForm const &form) const
{
    try
    {
        form.execute(*this);
        std::cout << _name << " executed " << form << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << _name << " couldn't execute " << form << " because " << e.what() << "." << std::endl;
    }
}

std::ostream &operator<<(std::ostream &out, const Bureaucrat &b)
{
    out << b.getName() << ", bureaucrat grade " << b.getGrade() << ".";
    return out;
}
