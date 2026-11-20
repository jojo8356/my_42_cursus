#include "Dog.hpp"

Dog::Dog(void) : Animal("Dog"), _brain(new Brain())
{
    std::cout << "Dog default constructor called" << std::endl;
}

Dog::Dog(const Dog &src) : Animal(src), _brain(new Brain(*src._brain))
{
    std::cout << "Dog copy constructor called" << std::endl;
}

Dog::~Dog(void)
{
    delete _brain;
    std::cout << "Dog destructor called" << std::endl;
}

Dog &Dog::operator=(const Dog &rhs)
{
    if (this != &rhs)
    {
        Animal::operator=(rhs);
        delete _brain;
        _brain = new Brain(*rhs._brain);
    }
    return *this;
}

void Dog::makeSound(void) const
{
    std::cout << "Woof" << std::endl;
}

Brain *Dog::getBrain(void) const
{
    return _brain;
}
