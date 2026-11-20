#ifndef ROBOTOMYREQUESTFORM_HPP
#define ROBOTOMYREQUESTFORM_HPP

#include "AForm.hpp"

class RobotomyRequestForm : public AForm
{
    public:
        RobotomyRequestForm(const std::string &target);

        virtual ~RobotomyRequestForm(void);
        std::string getTarget(void) const;

    protected:
        void executeAction(Bureaucrat const &executor) const;

    private:
        const std::string _target;
};

#endif
