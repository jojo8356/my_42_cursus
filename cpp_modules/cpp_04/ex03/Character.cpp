#include "Character.hpp"

Character::Character(void) : _name("")
{
    for (int i = 0; i < 4; ++i)
        _inventory[i] = NULL;
}

Character::Character(const std::string &name) : _name(name)
{
    for (int i = 0; i < 4; ++i)
        _inventory[i] = NULL;
}

Character::Character(const Character &src) : _name(src._name)
{
    for (int i = 0; i < 4; ++i)
    {
        if (src._inventory[i])
            _inventory[i] = src._inventory[i]->clone();
        else
            _inventory[i] = NULL;
    }
}

Character::~Character(void)
{
    for (int i = 0; i < 4; ++i)
        delete _inventory[i];
}

Character &Character::operator=(const Character &rhs)
{
    if (this != &rhs)
    {
        _name = rhs._name;
        for (int i = 0; i < 4; ++i)
        {
            delete _inventory[i];
            _inventory[i] = rhs._inventory[i] ? rhs._inventory[i]->clone() : NULL;
        }
    }
    return *this;
}

std::string const &Character::getName() const
{
    return _name;
}

void Character::equip(AMateria *m)
{
    if (!m)
        return;
    for (int i = 0; i < 4; ++i)
    {
        if (_inventory[i] == NULL)
        {
            _inventory[i] = m->clone();
            return;
        }
    }
}

void Character::unequip(int idx)
{
    if (idx < 0 || idx >= 4 || _inventory[idx] == NULL)
        return;
    _inventory[idx] = NULL;
}

void Character::use(int idx, ICharacter &target)
{
    if (idx < 0 || idx >= 4 || _inventory[idx] == NULL)
        return;
    _inventory[idx]->use(target);
}
