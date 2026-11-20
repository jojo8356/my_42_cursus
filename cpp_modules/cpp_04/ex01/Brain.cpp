#include "Brain.hpp"

Brain::Brain(void)
{
    std::cout << "Brain default constructor called" << std::endl;
}

Brain::Brain(const Brain &src)
{
    for (int i = 0; i < 100; ++i)
        _ideas[i] = src._ideas[i];
    std::cout << "Brain copy constructor called" << std::endl;
}

Brain::~Brain(void)
{
    std::cout << "Brain destructor called" << std::endl;
}

Brain &Brain::operator=(const Brain &rhs)
{
    if (this != &rhs)
    {
        for (int i = 0; i < 100; ++i)
            _ideas[i] = rhs._ideas[i];
    }
    std::cout << "Brain copy assignment operator called" << std::endl;
    return *this;
}

std::string Brain::getIdea(int index) const
{
    return _ideas[index];
}

void Brain::setIdea(int index, const std::string &idea)
{
    _ideas[index] = idea;
}
