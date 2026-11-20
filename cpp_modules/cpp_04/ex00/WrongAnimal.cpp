#include "WrongAnimal.hpp"

WrongAnimal::WrongAnimal(void) : _type("WrongAnimal")
{
    std::cout << "WrongAnimal default constructor called" << std::endl;
}

WrongAnimal::WrongAnimal(const std::string &type) : _type(type)
{
    std::cout << "WrongAnimal constructor called" << std::endl;
}

WrongAnimal::WrongAnimal(const WrongAnimal &src) : _type(src._type)
{
    std::cout << "WrongAnimal copy constructor called" << std::endl;
}

WrongAnimal::~WrongAnimal(void)
{
    std::cout << "WrongAnimal destructor called" << std::endl;
}

WrongAnimal &WrongAnimal::operator=(const WrongAnimal &rhs)
{
    if (this != &rhs)
        _type = rhs._type;
    return *this;
}

void WrongAnimal::makeSound(void) const
{
    std::cout << "...wrong animal sound..." << std::endl;
}

std::string WrongAnimal::getType(void) const
{
    return _type;
}
