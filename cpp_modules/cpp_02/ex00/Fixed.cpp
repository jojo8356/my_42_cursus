#include "Fixed.hpp"

const int Fixed::_fracBits;

Fixed::Fixed(void) : _value(0)
{
    std::cout << "Default constructor called" << std::endl;
}

Fixed::Fixed(const Fixed &src) : _value(src._value)
{
    std::cout << "Copy constructor called" << std::endl;
}

Fixed::~Fixed(void)
{
    std::cout << "Destructor called" << std::endl;
}

Fixed &Fixed::operator=(const Fixed &rhs)
{
    if (this != &rhs)
        _value = rhs._value;
    std::cout << "Copy assignment operator called" << std::endl;
    return *this;
}

int Fixed::getRawBits(void) const
{
    std::cout << "getRawBits member function called" << std::endl;
    return _value;
}

void Fixed::setRawBits(int const raw)
{
    _value = raw;
}
