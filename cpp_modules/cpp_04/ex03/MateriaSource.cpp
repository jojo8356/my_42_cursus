#include "MateriaSource.hpp"

MateriaSource::MateriaSource(void)
{
    for (int i = 0; i < 4; ++i)
        _learned[i] = NULL;
}

MateriaSource::~MateriaSource(void)
{
    for (int i = 0; i < 4; ++i)
        delete _learned[i];
}

void MateriaSource::learnMateria(AMateria *m)
{
    if (!m)
        return;
    for (int i = 0; i < 4; ++i)
    {
        if (_learned[i] == NULL)
        {
            _learned[i] = m->clone();
            return;
        }
    }
}

AMateria *MateriaSource::createMateria(std::string const &type)
{
    for (int i = 0; i < 4; ++i)
    {
        if (_learned[i] && _learned[i]->getType() == type)
            return _learned[i]->clone();
    }
    return NULL;
}
