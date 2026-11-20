#include "Fixed.hpp"
#include <cmath>

const int Fixed::_fracBits;

Fixed::Fixed(void) : _value(0)
{
    std::cout << "Default constructor called" << std::endl;
}

Fixed::Fixed(int const nbr) : _value(nbr << _fracBits)
{
    std::cout << "Int constructor called" << std::endl;
}

Fixed::Fixed(float const nbr) : _value(static_cast<int>(roundf(nbr * (1 << _fracBits))))
{
    std::cout << "Float constructor called" << std::endl;
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

float Fixed::toFloat(void) const
{
    return static_cast<float>(_value) / (1 << _fracBits);
}

int Fixed::toInt(void) const
{
    return static_cast<int>(roundf(toFloat()));
}

std::ostream &operator<<(std::ostream &out, const Fixed &fixed)
{
    out << fixed.toFloat();
    return out;
}
