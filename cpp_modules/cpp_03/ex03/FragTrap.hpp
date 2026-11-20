#ifndef FRAGTRAP_HPP
#define FRAGTRAP_HPP

#include "ClapTrap.hpp"

class FragTrap : public virtual ClapTrap
{
    public:
        FragTrap(void);
        FragTrap(const std::string &name);
        FragTrap(const FragTrap &src);
        ~FragTrap(void);

        FragTrap &operator=(const FragTrap &rhs);

        void attack(const std::string &target);
        void highFivesGuys(void) const;
};

#endif
