#ifndef PRESIDENTIALPARDONFORM_HPP
#define PRESIDENTIALPARDONFORM_HPP

#include "AForm.hpp"

class PresidentialPardonForm : public AForm
{
    public:
        PresidentialPardonForm(const std::string &target);

        virtual ~PresidentialPardonForm(void);
        std::string getTarget(void) const;

    protected:
        void executeAction(Bureaucrat const &executor) const;

    private:
        const std::string _target;
};

#endif
