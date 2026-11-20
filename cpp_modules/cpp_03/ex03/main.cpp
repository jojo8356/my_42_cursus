#include "ClapTrap.hpp"
#include "ScavTrap.hpp"
#include "FragTrap.hpp"
#include "DiamondTrap.hpp"

int main(void)
{
    {
        DiamondTrap diamond("DI-42");
        diamond.attack("target");
        diamond.takeDamage(30);
        diamond.beRepaired(10);
        diamond.guardGate();
        diamond.highFivesGuys();
    }
    return 0;
}
