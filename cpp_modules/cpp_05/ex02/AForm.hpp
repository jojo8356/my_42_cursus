#ifndef AFORM_HPP
#define AFORM_HPP

#include <iostream>
#include <string>
#include <exception>
#include "Bureaucrat.hpp"

class AForm
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

        class SignatureNeededException : public std::exception
        {
            public:
                const char *what() const throw();
        };

        AForm(const std::string &name, int gradeSign, int gradeExec);
        AForm(const AForm &src);
        virtual ~AForm(void);

        AForm &operator=(const AForm &rhs);

        std::string getName(void) const;
        bool getSigned(void) const;
        int getGradeSign(void) const;
        int getGradeExec(void) const;

        void beSigned(const Bureaucrat &b);
        virtual void execute(Bureaucrat const &executor) const;

    private:
        const std::string _name;
        bool _signed;
        const int _gradeSign;
        const int _gradeExec;

    protected:
        virtual void executeAction(Bureaucrat const &executor) const = 0;
};

std::ostream &operator<<(std::ostream &out, const AForm &f);

#endif
