#ifndef DIAMONDTRAP_HPP
#define DIAMONDTRAP_HPP

#include "FragTrap.hpp"
#include "ScavTrap.hpp"

class DiamondTrap : public FragTrap, public ScavTrap
{
    public:
        DiamondTrap(void);
        DiamondTrap(const std::string &name);
        DiamondTrap(const DiamondTrap &src);
        ~DiamondTrap(void);

        DiamondTrap &operator=(const DiamondTrap &rhs);

        void attack(const std::string &target);
        void takeDamage(unsigned int amount);
        void beRepaired(unsigned int amount);
        void guardGate(void) const;
        void highFivesGuys(void) const;
};

#endif
