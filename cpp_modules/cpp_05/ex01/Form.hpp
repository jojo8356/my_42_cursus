#ifndef FORM_HPP
#define FORM_HPP

#include <iostream>
#include <string>
#include <exception>
#include "Bureaucrat.hpp"

class Form
{
    public:
        class GradeTooHighException : public std::exception
        {
            public:
                const char *what() const throw();
        };

        class GradeTooLowException : public std::exception
        {
            public:
                const char *what() const throw();
        };

        Form(void);
        Form(const std::string &name, int gradeSign, int gradeExec);
        Form(const Form &src);
        ~Form(void);

        Form &operator=(const Form &rhs);

        std::string getName(void) const;
        bool getSigned(void) const;
        int getGradeSign(void) const;
        int getGradeExec(void) const;

        void beSigned(const Bureaucrat &b);

    private:
        const std::string _name;
        bool _signed;
        const int _gradeSign;
        const int _gradeExec;
};

std::ostream &operator<<(std::ostream &out, const Form &f);

#endif
