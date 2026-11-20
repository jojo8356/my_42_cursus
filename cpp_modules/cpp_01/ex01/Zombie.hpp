#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include <string>
#include <iostream>

class Zombie
{
    public:
        Zombie(void);
        Zombie(const std::string &name);
        ~Zombie(void);

        void announce(void) const;
        void setName(const std::string &name);

    private:
        std::string _name;
};

Zombie *zombieHorde(int N, std::string name);

#endif
