#include "ClapTrap.hpp"
#include "ScavTrap.hpp"

int main(void)
{
    {
        ClapTrap clap("CL-42");
        ScavTrap scav("SC-42");
        scav.attack("target");
        scav.guardGate();
        clap.attack("target");
    }
    return 0;
}
