#ifndef MATERIASOURCE_HPP
#define MATERIASOURCE_HPP

#include <iostream>
#include "IMateriaSource.hpp"
#include "AMateria.hpp"

class MateriaSource : public IMateriaSource
{
    public:
        MateriaSource(void);

        virtual ~MateriaSource(void);
        void learnMateria(AMateria *m);
        AMateria *createMateria(std::string const &type);

    private:
        AMateria *_learned[4];
};

#endif
