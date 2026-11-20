#include "Point.hpp"

Point::Point(void) : x(0), y(0)
{
}

Point::Point(float const px, float const py) : x(px), y(py)
{
}

Point::Point(const Point &src) : x(src.x), y(src.y)
{
}

Point::~Point(void)
{
}

Point &Point::operator=(const Point &rhs)
{
    if (this != &rhs)
    {
        // x and y are const members and cannot be reassigned.
    }
    return *this;
}

Fixed const &Point::getX(void) const
{
    return x;
}

Fixed const &Point::getY(void) const
{
    return y;
}
