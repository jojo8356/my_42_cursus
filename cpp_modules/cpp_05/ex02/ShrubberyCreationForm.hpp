#ifndef SHRUBBERYCREATIONFORM_HPP
#define SHRUBBERYCREATIONFORM_HPP

#include "AForm.hpp"

class ShrubberyCreationForm : public AForm
{
    public:
        ShrubberyCreationForm(const std::string &target);

        virtual ~ShrubberyCreationForm(void);
        std::string getTarget(void) const;

    protected:
        void executeAction(Bureaucrat const &executor) const;

    private:
        const std::string _target;
};

#endif
