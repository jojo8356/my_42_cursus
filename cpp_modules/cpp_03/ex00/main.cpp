#include "ClapTrap.hpp"

int main(void)
{
    ClapTrap clap("CL-42");
    clap.attack("target");
    clap.takeDamage(5);
    clap.beRepaired(5);
    clap.attack("target");
    clap.takeDamage(20);
    clap.attack("target");
    return 0;
}
