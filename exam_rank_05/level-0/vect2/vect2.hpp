/*
 * vect2.hpp — 42 Exam Rank 05 (new 2025), level 0
 * A simple 2D vector class with arithmetic operators (C++98).
 */
#ifndef VECT2_HPP
#define VECT2_HPP

#include <iostream>

class vect2
{
public:
    int x;
    int y;

    vect2();
    vect2(int x, int y);

    vect2(const vect2 &other);
    vect2 &operator=(const vect2 &other);
    ~vect2();

    int &operator[](int index);
    const int &operator[](int index) const;

    vect2 &operator+=(const vect2 &other);
    vect2 &operator-=(const vect2 &other);
    vect2 &operator*=(int scalar);
    vect2 &operator/=(int scalar);

    vect2 operator+(const vect2 &other) const;
    vect2 operator-(const vect2 &other) const;
    vect2 operator*(int scalar) const;
    vect2 operator/(int scalar) const;

    vect2 &operator++();
    vect2 operator++(int);
    vect2 &operator--();
    vect2 operator--(int);

    vect2 operator-() const;

    bool operator==(const vect2 &other) const;
    bool operator!=(const vect2 &other) const;
};

std::ostream &operator<<(std::ostream &os, const vect2 &v);

#endif /* VECT2_HPP */
