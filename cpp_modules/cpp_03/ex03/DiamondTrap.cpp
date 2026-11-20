#include "DiamondTrap.hpp"

DiamondTrap::DiamondTrap(void) : ClapTrap(), FragTrap(), ScavTrap()
{
    std::cout << "DiamondTrap default constructor called" << std::endl;
}

DiamondTrap::DiamondTrap(const std::string &name) : ClapTrap(name, 100, 100, 30), FragTrap(name), ScavTrap(name)
{
    std::cout << "DiamondTrap " << _name << " is constructed!" << std::endl;
}

DiamondTrap::DiamondTrap(const DiamondTrap &src) : ClapTrap(src), FragTrap(src), ScavTrap(src)
{
    std::cout << "DiamondTrap copy constructor called" << std::endl;
}

DiamondTrap::~DiamondTrap(void)
{
    std::cout << "DiamondTrap " << _name << " is destroyed!" << std::endl;
}

DiamondTrap &DiamondTrap::operator=(const DiamondTrap &rhs)
{
    if (this != &rhs)
    {
        FragTrap::operator=(rhs);
        ScavTrap::operator=(rhs);
    }
    return *this;
}

void DiamondTrap::attack(const std::string &target)
{
    if (_hitPoints <= 0 || _energyPoints <= 0)
    {
        std::cout << "DiamondTrap " << _name << " is not able to attack." << std::endl;
        return;
    }
    std::cout << "DiamondTrap " << _name << " attacks " << target
              << ", causing " << _attackDamage << " points of damage!" << std::endl;
    --_energyPoints;
}

void DiamondTrap::takeDamage(unsigned int amount)
{
    if (_hitPoints <= 0)
    {
        std::cout << "DiamondTrap " << _name << " is already dead." << std::endl;
        return;
    }
    std::cout << "DiamondTrap " << _name << " takes " << amount << " points of damage!" << std::endl;
    if (static_cast<int>(amount) >= _hitPoints)
        _hitPoints = 0;
    else
        _hitPoints -= static_cast<int>(amount);
}

void DiamondTrap::beRepaired(unsigned int amount)
{
    if (_hitPoints <= 0 || _energyPoints <= 0)
    {
        std::cout << "DiamondTrap " << _name << " is not able to repair." << std::endl;
        return;
    }
    std::cout << "DiamondTrap " << _name << " repairs " << amount << " hit points!" << std::endl;
    _hitPoints += static_cast<int>(amount);
    --_energyPoints;
}

void DiamondTrap::guardGate(void) const
{
    std::cout << "DiamondTrap " << _name << " is now in Gate keeper mode." << std::endl;
}

void DiamondTrap::highFivesGuys(void) const
{
    std::cout << "DiamondTrap " << _name << " requests high fives!" << std::endl;
}
