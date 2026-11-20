#include "Animal.hpp"

Animal::Animal(void) : _type("Animal")
{
    std::cout << "Animal default constructor called" << std::endl;
}

Animal::Animal(const std::string &type) : _type(type)
{
    std::cout << "Animal constructor called" << std::endl;
}

Animal::Animal(const Animal &src) : _type(src._type)
{
    std::cout << "Animal copy constructor called" << std::endl;
}

Animal::~Animal(void)
{
    std::cout << "Animal destructor called" << std::endl;
}

Animal &Animal::operator=(const Animal &rhs)
{
    if (this != &rhs)
        _type = rhs._type;
    return *this;
}

std::string Animal::getType(void) const
{
    return _type;
}
