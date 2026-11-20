#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <iostream>
#include <string>
#include "ICharacter.hpp"
#include "AMateria.hpp"

class Character : public ICharacter
{
    public:
        Character(void);
        Character(const std::string &name);
        Character(const Character &src);

        Character &operator=(const Character &rhs);

        virtual ~Character(void);
        std::string const &getName() const;
        void equip(AMateria *m);
        void unequip(int idx);
        void use(int idx, ICharacter &target);

    private:
        std::string _name;
        AMateria *_inventory[4];
};

#endif
