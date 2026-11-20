#include "Zombie.hpp"
#include <cstdlib>

Zombie *newZombie(std::string name)
{
    return new Zombie(name);
}
