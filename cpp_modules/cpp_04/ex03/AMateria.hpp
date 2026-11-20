#ifndef AMATERIA_HPP
#define AMATERIA_HPP

#include <iostream>
#include <string>
#include "ICharacter.hpp"

class AMateria
{
    public:
        AMateria(std::string const &type);
        AMateria(const AMateria &src);
        virtual ~AMateria(void);

        AMateria &operator=(const AMateria &rhs);

        std::string const &getType(void) const;

        virtual AMateria *clone(void) const = 0;
        virtual void use(ICharacter &target);

    protected:
        std::string _type;
};

#endif
