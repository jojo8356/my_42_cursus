#include "AMateria.hpp"

AMateria::AMateria(std::string const &type) : _type(type)
{
}

AMateria::AMateria(const AMateria &src) : _type(src._type)
{
}

AMateria::~AMateria(void)
{
}

AMateria &AMateria::operator=(const AMateria &rhs)
{
    if (this != &rhs)
        _type = rhs._type;
    return *this;
}

std::string const &AMateria::getType(void) const
{
    return _type;
}

void AMateria::use(ICharacter &target)
{
    (void)target;
}
