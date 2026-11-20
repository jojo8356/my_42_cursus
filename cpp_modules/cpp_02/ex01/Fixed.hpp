#ifndef FIXED_HPP
#define FIXED_HPP

#include <iostream>

class Fixed
{
    public:
        Fixed(void);
        Fixed(int const nbr);
        Fixed(float const nbr);
        Fixed(const Fixed &src);
        ~Fixed(void);

        Fixed &operator=(const Fixed &rhs);

        int getRawBits(void) const;
        void setRawBits(int const raw);

        float toFloat(void) const;
        int toInt(void) const;

    private:
        int _value;
        static const int _fracBits = 8;
};

std::ostream &operator<<(std::ostream &out, const Fixed &fixed);

#endif
