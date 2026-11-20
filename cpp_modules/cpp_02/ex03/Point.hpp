#ifndef POINT_HPP
#define POINT_HPP

#include "Fixed.hpp"

class Point
{
    public:
        Point(void);
        Point(float const x, float const y);
        Point(const Point &src);
        ~Point(void);

        Point &operator=(const Point &rhs);

        Fixed const &getX(void) const;
        Fixed const &getY(void) const;

    private:
        Fixed const x;
        Fixed const y;
};

bool bsp(Point const a, Point const b, Point const c, Point const point);

#endif
