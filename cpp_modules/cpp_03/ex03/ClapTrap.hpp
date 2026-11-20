#ifndef CLAPTRAP_HPP
#define CLAPTRAP_HPP

#include <iostream>
#include <string>

class ClapTrap
{
    public:
        ClapTrap(void);
        ClapTrap(const std::string &name);
        ClapTrap(const std::string &name, int hitPoints, int energyPoints, int attackDamage);
        ClapTrap(const ClapTrap &src);
        ~ClapTrap(void);

        ClapTrap &operator=(const ClapTrap &rhs);

        void attack(const std::string &target);
        void takeDamage(unsigned int amount);
        void beRepaired(unsigned int amount);

        std::string getName(void) const;

    protected:
        std::string _name;
        int _hitPoints;
        int _energyPoints;
        int _attackDamage;
};

#endif
