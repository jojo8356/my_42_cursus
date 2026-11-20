#include "FragTrap.hpp"

FragTrap::FragTrap(void) : ClapTrap("", 100, 100, 30)
{
    std::cout << "FragTrap default constructor called" << std::endl;
}

FragTrap::FragTrap(const std::string &name) : ClapTrap(name, 100, 100, 30)
{
    std::cout << "FragTrap " << _name << " is constructed!" << std::endl;
}

FragTrap::FragTrap(const FragTrap &src) : ClapTrap(src)
{
    std::cout << "FragTrap copy constructor called" << std::endl;
}

FragTrap::~FragTrap(void)
{
    std::cout << "FragTrap " << _name << " is destroyed!" << std::endl;
}

FragTrap &FragTrap::operator=(const FragTrap &rhs)
{
    if (this != &rhs)
        ClapTrap::operator=(rhs);
    return *this;
}

void FragTrap::attack(const std::string &target)
{
    if (_hitPoints <= 0 || _energyPoints <= 0)
    {
        std::cout << "FragTrap " << _name << " is not able to attack." << std::endl;
        return;
    }
    std::cout << "FragTrap " << _name << " attacks " << target
              << ", causing " << _attackDamage << " points of damage!" << std::endl;
    --_energyPoints;
}

void FragTrap::highFivesGuys(void) const
{
    std::cout << "FragTrap " << _name << " requests high fives!" << std::endl;
}
