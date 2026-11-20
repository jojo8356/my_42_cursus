#include "ScavTrap.hpp"

ScavTrap::ScavTrap(void) : ClapTrap("", 100, 50, 20)
{
    std::cout << "ScavTrap default constructor called" << std::endl;
}

ScavTrap::ScavTrap(const std::string &name) : ClapTrap(name, 100, 50, 20)
{
    std::cout << "ScavTrap " << _name << " is constructed!" << std::endl;
}

ScavTrap::ScavTrap(const ScavTrap &src) : ClapTrap(src)
{
    std::cout << "ScavTrap copy constructor called" << std::endl;
}

ScavTrap::~ScavTrap(void)
{
    std::cout << "ScavTrap " << _name << " is destroyed!" << std::endl;
}

ScavTrap &ScavTrap::operator=(const ScavTrap &rhs)
{
    if (this != &rhs)
        ClapTrap::operator=(rhs);
    return *this;
}

void ScavTrap::attack(const std::string &target)
{
    if (_hitPoints <= 0 || _energyPoints <= 0)
    {
        std::cout << "ScavTrap " << _name << " is not able to attack." << std::endl;
        return;
    }
    std::cout << "ScavTrap " << _name << " attacks " << target
              << ", causing " << _attackDamage << " points of damage!" << std::endl;
    --_energyPoints;
}

void ScavTrap::guardGate(void) const
{
    std::cout << "ScavTrap " << _name << " is now in Gate keeper mode." << std::endl;
}
