#include "Fixed.hpp"
#include <cmath>

const int Fixed::_fracBits;

Fixed::Fixed(void) : _value(0)
{
}

Fixed::Fixed(int const nbr) : _value(nbr << _fracBits)
{
}

Fixed::Fixed(float const nbr) : _value(static_cast<int>(roundf(nbr * (1 << _fracBits))))
{
}

Fixed::Fixed(const Fixed &src) : _value(src._value)
{
}

Fixed::~Fixed(void)
{
}

Fixed &Fixed::operator=(const Fixed &rhs)
{
    if (this != &rhs)
        _value = rhs._value;
    return *this;
}

int Fixed::getRawBits(void) const
{
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

bool Fixed::operator>(const Fixed &rhs) const
{
    return _value > rhs._value;
}

bool Fixed::operator<(const Fixed &rhs) const
{
    return _value < rhs._value;
}

bool Fixed::operator>=(const Fixed &rhs) const
{
    return _value >= rhs._value;
}

bool Fixed::operator<=(const Fixed &rhs) const
{
    return _value <= rhs._value;
}

bool Fixed::operator==(const Fixed &rhs) const
{
    return _value == rhs._value;
}

bool Fixed::operator!=(const Fixed &rhs) const
{
    return _value != rhs._value;
}

Fixed Fixed::operator+(const Fixed &rhs) const
{
    return Fixed(static_cast<float>(_value + rhs._value) / (1 << _fracBits));
}

Fixed Fixed::operator-(const Fixed &rhs) const
{
    return Fixed(static_cast<float>(_value - rhs._value) / (1 << _fracBits));
}

Fixed Fixed::operator*(const Fixed &rhs) const
{
    return Fixed(toFloat() * rhs.toFloat());
}

Fixed Fixed::operator/(const Fixed &rhs) const
{
    return Fixed(toFloat() / rhs.toFloat());
}

Fixed &Fixed::operator++(void)
{
    ++_value;
    return *this;
}

Fixed Fixed::operator++(int)
{
    Fixed copy(*this);
    ++_value;
    return copy;
}

Fixed &Fixed::operator--(void)
{
    --_value;
    return *this;
}

Fixed Fixed::operator--(int)
{
    Fixed copy(*this);
    --_value;
    return copy;
}

Fixed &Fixed::min(Fixed &a, Fixed &b)
{
    return a < b ? a : b;
}

Fixed const &Fixed::min(Fixed const &a, Fixed const &b)
{
    return a < b ? a : b;
}

Fixed &Fixed::max(Fixed &a, Fixed &b)
{
    return a > b ? a : b;
}

Fixed const &Fixed::max(Fixed const &a, Fixed const &b)
{
    return a > b ? a : b;
}

std::ostream &operator<<(std::ostream &out, const Fixed &fixed)
{
    out << fixed.toFloat();
    return out;
}
