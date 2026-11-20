#ifndef INTERN_HPP
#define INTERN_HPP

#include <iostream>
#include <string>
#include "AForm.hpp"

class Intern
{
    public:
        Intern(void);
        Intern(const Intern &src);
        ~Intern(void);

        Intern &operator=(const Intern &rhs);

        AForm *makeForm(const std::string &name, const std::string &formType) const;
};

#endif
