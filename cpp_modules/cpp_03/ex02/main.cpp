#include "ClapTrap.hpp"
#include "ScavTrap.hpp"
#include "FragTrap.hpp"

int main(void)
{
    {
        ClapTrap clap("CL-42");
        ScavTrap scav("SC-42");
        FragTrap frag("FR-42");
        frag.attack("target");
        frag.highFivesGuys();
        scav.guardGate();
        clap.attack("target");
    }
    return 0;
}
