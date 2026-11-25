/*
 * bigint.hpp — 42 Exam Rank 05 (new 2025), level 0
 * An arbitrary-precision unsigned integer stored in base-10 digits (C++98).
 */
#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <iostream>
#include <string>

class bigint
{
public:
    std::string digits;

    bigint();
    bigint(int n);
    bigint(const std::string &s);

    bigint(const bigint &other);
    bigint &operator=(const bigint &other);
    ~bigint();

    // arithmetic
    bigint operator+(const bigint &other) const;
    bigint operator+(int n) const;
    bigint &operator+=(const bigint &other);

    bigint operator-(const bigint &other) const;
    bigint &operator-=(const bigint &other);

    bigint operator*(const bigint &other) const;
    bigint operator*(int n) const;

    bigint operator/(int n) const;
    bigint operator%(int n) const;

    // digitshift (base-10)
    bigint operator<<(int n) const;
    bigint operator>>(int n) const;
    bigint &operator<<=(int n);
    bigint &operator>>=(const bigint &n);

    // increment / decrement
    bigint &operator++();
    bigint operator++(int);
    bigint &operator--();
    bigint operator--(int);

    // comparison
    bool operator<(const bigint &other) const;
    bool operator<=(const bigint &other) const;
    bool operator>(const bigint &other) const;
    bool operator>=(const bigint &other) const;
    bool operator==(const bigint &other) const;
    bool operator!=(const bigint &other) const;

    void normalize();

private:
    static std::string int_to_str(int n);
    static int str_to_int(const std::string &s);
};

std::ostream &operator<<(std::ostream &os, const bigint &b);

#endif /* BIGINT_HPP */
