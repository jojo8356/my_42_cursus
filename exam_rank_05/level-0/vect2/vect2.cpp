/*
 * vect2.cpp — 42 Exam Rank 05 (new 2025), level 0
 */
#include "vect2.hpp"

vect2::vect2() : x(0), y(0) {}

vect2::vect2(int x, int y) : x(x), y(y) {}

vect2::vect2(const vect2 &other) : x(other.x), y(other.y) {}

vect2 &vect2::operator=(const vect2 &other)
{
    if (this != &other)
    {
        x = other.x;
        y = other.y;
    }
    return *this;
}

vect2::~vect2() {}

int &vect2::operator[](int index)
{
    if (index == 0)
        return x;
    return y;
}

const int &vect2::operator[](int index) const
{
    if (index == 0)
        return x;
    return y;
}

vect2 &vect2::operator+=(const vect2 &other)
{
    x += other.x;
    y += other.y;
    return *this;
}

vect2 &vect2::operator-=(const vect2 &other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

vect2 &vect2::operator*=(int scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

vect2 &vect2::operator/=(int scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

vect2 vect2::operator+(const vect2 &other) const
{
    return vect2(x + other.x, y + other.y);
}

vect2 vect2::operator-(const vect2 &other) const
{
    return vect2(x - other.x, y - other.y);
}

vect2 vect2::operator*(int scalar) const
{
    return vect2(x * scalar, y * scalar);
}

vect2 vect2::operator/(int scalar) const
{
    return vect2(x / scalar, y / scalar);
}

vect2 &vect2::operator++()
{
    x++;
    y++;
    return *this;
}

vect2 vect2::operator++(int)
{
    vect2 copy(*this);
    x++;
    y++;
    return copy;
}

vect2 &vect2::operator--()
{
    x--;
    y--;
    return *this;
}

vect2 vect2::operator--(int)
{
    vect2 copy(*this);
    x--;
    y--;
    return copy;
}

vect2 vect2::operator-() const
{
    return vect2(-x, -y);
}

bool vect2::operator==(const vect2 &other) const
{
    return x == other.x && y == other.y;
}

bool vect2::operator!=(const vect2 &other) const
{
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const vect2 &v)
{
    os << "{" << v.x << ", " << v.y << "}";
    return os;
}
