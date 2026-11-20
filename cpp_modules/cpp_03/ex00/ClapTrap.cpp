#include "ClapTrap.hpp"

ClapTrap::ClapTrap(void) : _name(""), _hitPoints(10), _energyPoints(10), _attackDamage(0)
{
    std::cout << "ClapTrap default constructor called" << std::endl;
}

ClapTrap::ClapTrap(const std::string &name)
    : _name(name), _hitPoints(10), _energyPoints(10), _attackDamage(0)
{
    std::cout << "ClapTrap " << _name << " is constructed!" << std::endl;
}

ClapTrap::ClapTrap(const std::string &name, int hitPoints, int energyPoints, int attackDamage)
    : _name(name), _hitPoints(hitPoints), _energyPoints(energyPoints), _attackDamage(attackDamage)
{
    std::cout << "ClapTrap " << _name << " is constructed!" << std::endl;
}

ClapTrap::ClapTrap(const ClapTrap &src)
    : _name(src._name), _hitPoints(src._hitPoints), _energyPoints(src._energyPoints), _attackDamage(src._attackDamage)
{
    std::cout << "ClapTrap copy constructor called" << std::endl;
}

ClapTrap::~ClapTrap(void)
{
    std::cout << "ClapTrap " << _name << " is destroyed!" << std::endl;
}

ClapTrap &ClapTrap::operator=(const ClapTrap &rhs)
{
    if (this != &rhs)
    {
        _name = rhs._name;
        _hitPoints = rhs._hitPoints;
        _energyPoints = rhs._energyPoints;
        _attackDamage = rhs._attackDamage;
    }
    return *this;
}

void ClapTrap::attack(const std::string &target)
{
    if (_hitPoints <= 0 || _energyPoints <= 0)
    {
        std::cout << "ClapTrap " << _name << " is not able to attack." << std::endl;
        return;
    }
    std::cout << "ClapTrap " << _name << " attacks " << target
              << ", causing " << _attackDamage << " points of damage!" << std::endl;
    --_energyPoints;
}

void ClapTrap::takeDamage(unsigned int amount)
{
    if (_hitPoints <= 0)
    {
        std::cout << "ClapTrap " << _name << " is already dead." << std::endl;
        return;
    }
    std::cout << "ClapTrap " << _name << " takes " << amount << " points of damage!" << std::endl;
    if (static_cast<int>(amount) >= _hitPoints)
        _hitPoints = 0;
    else
        _hitPoints -= static_cast<int>(amount);
}

void ClapTrap::beRepaired(unsigned int amount)
{
    if (_hitPoints <= 0 || _energyPoints <= 0)
    {
        std::cout << "ClapTrap " << _name << " is not able to repair." << std::endl;
        return;
    }
    std::cout << "ClapTrap " << _name << " repairs " << amount << " hit points!" << std::endl;
    _hitPoints += static_cast<int>(amount);
    --_energyPoints;
}

std::string ClapTrap::getName(void) const
{
    return _name;
}
