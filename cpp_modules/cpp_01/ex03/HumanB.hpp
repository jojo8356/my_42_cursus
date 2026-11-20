#ifndef HUMNB_HPP
#define HUMNB_HPP

#include <string>
#include "Weapon.hpp"

class HumanB
{
    public:
        HumanB(const std::string &name);
        ~HumanB(void);

        void setWeapon(const Weapon &weapon);
        void attack(void) const;

    private:
        std::string _name;
        const Weapon *_weapon;
};

#endif
