#include "MateriaSource.hpp"
#include "Character.hpp"
#include "Ice.hpp"
#include "Cure.hpp"

int main(void)
{
    MateriaSource source;
    source.learnMateria(new Ice());
    source.learnMateria(new Cure());

    Character me("Inna");
    Character *tmp = new Character("Bob");

    AMateria *a = source.createMateria("ice");
    AMateria *b = source.createMateria("cure");
    AMateria *c = source.createMateria("unknown");

    me.equip(a);
    me.equip(b);
    me.use(0, *tmp);
    me.use(1, *tmp);
    me.unequip(0);
    me.use(0, *tmp);

    if (c == NULL)
        std::cout << "createMateria unknown returned NULL as expected" << std::endl;

    delete a;
    delete b;
    delete tmp;
    return 0;
}
