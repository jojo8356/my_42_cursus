#include "Cat.hpp"

Cat::Cat(void) : Animal("Cat"), _brain(new Brain())
{
    std::cout << "Cat default constructor called" << std::endl;
}

Cat::Cat(const Cat &src) : Animal(src), _brain(new Brain(*src._brain))
{
    std::cout << "Cat copy constructor called" << std::endl;
}

Cat::~Cat(void)
{
    delete _brain;
    std::cout << "Cat destructor called" << std::endl;
}

Cat &Cat::operator=(const Cat &rhs)
{
    if (this != &rhs)
    {
        Animal::operator=(rhs);
        delete _brain;
        _brain = new Brain(*rhs._brain);
    }
    return *this;
}

void Cat::makeSound(void) const
{
    std::cout << "Meow" << std::endl;
}

Brain *Cat::getBrain(void) const
{
    return _brain;
}
