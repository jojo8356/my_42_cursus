#include "Point.hpp"

bool bsp(Point const a, Point const b, Point const c, Point const point)
{
    float ax = a.getX().toFloat();
    float ay = a.getY().toFloat();
    float bx = b.getX().toFloat();
    float by = b.getY().toFloat();
    float cx = c.getX().toFloat();
    float cy = c.getY().toFloat();
    float px = point.getX().toFloat();
    float py = point.getY().toFloat();

    float s1 = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    float s2 = (cx - bx) * (py - by) - (cy - by) * (px - bx);
    float s3 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx);

    if (s1 == 0.0f || s2 == 0.0f || s3 == 0.0f)
        return false;

    return (s1 > 0 && s2 > 0 && s3 > 0) || (s1 < 0 && s2 < 0 && s3 < 0);
}
