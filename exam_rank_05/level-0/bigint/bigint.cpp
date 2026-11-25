/*
 * bigint.cpp — 42 Exam Rank 05 (new 2025), level 0
 */
#include "bigint.hpp"

std::string bigint::int_to_str(int n)
{
    if (n == 0)
        return "0";
    std::string s;
    bool neg = false;
    if (n < 0)
    {
        neg = true;
        n = -n;
    }
    while (n > 0)
    {
        s = char('0' + (n % 10)) + s;
        n /= 10;
    }
    if (neg)
        s = "-" + s;
    return s;
}

int bigint::str_to_int(const std::string &s)
{
    int n = 0;
    size_t i = 0;
    if (s.size() > 0 && s[0] == '-')
        i = 1;
    for (; i < s.size(); i++)
        n = n * 10 + (s[i] - '0');
    if (s.size() > 0 && s[0] == '-')
        n = -n;
    return n;
}

bigint::bigint() : digits("0") {}

bigint::bigint(int n) : digits(int_to_str(n)) {}

bigint::bigint(const std::string &s) : digits(s)
{
    normalize();
}

bigint::bigint(const bigint &other) : digits(other.digits) {}

bigint &bigint::operator=(const bigint &other)
{
    if (this != &other)
        digits = other.digits;
    return *this;
}

bigint::~bigint() {}

void bigint::normalize()
{
    size_t i = 0;
    while (i < digits.size() && digits[i] == '0')
        i++;
    if (i == digits.size())
    {
        digits = "0";
        return;
    }
    digits = digits.substr(i);
    if (digits.empty())
        digits = "0";
}

bigint bigint::operator+(const bigint &other) const
{
    std::string a = digits;
    std::string b = other.digits;
    std::string result;
    int carry = 0;
    size_t i = a.size();
    size_t j = b.size();
    while (i > 0 || j > 0 || carry > 0)
    {
        int da = (i > 0) ? (a[i - 1] - '0') : 0;
        int db = (j > 0) ? (b[j - 1] - '0') : 0;
        int sum = da + db + carry;
        result = char('0' + (sum % 10)) + result;
        carry = sum / 10;
        if (i > 0)
            i--;
        if (j > 0)
            j--;
    }
    bigint r(result);
    r.normalize();
    return r;
}

bigint bigint::operator+(int n) const
{
    return *this + bigint(n);
}

bigint &bigint::operator+=(const bigint &other)
{
    digits = (*this + other).digits;
    normalize();
    return *this;
}

bigint bigint::operator-(const bigint &other) const
{
    // unsigned: result is 0 if *this < other.
    if (*this < other)
        return bigint(0);

    std::string a = digits;
    std::string b = other.digits;
    std::string result;
    int borrow = 0;
    size_t i = a.size();
    size_t j = b.size();
    while (i > 0)
    {
        int da = a[i - 1] - '0' - borrow;
        int db = (j > 0) ? (b[j - 1] - '0') : 0;
        if (da < db)
        {
            da += 10;
            borrow = 1;
        }
        else
        {
            borrow = 0;
        }
        result = char('0' + (da - db)) + result;
        i--;
        if (j > 0)
            j--;
    }
    bigint r(result);
    r.normalize();
    return r;
}

bigint &bigint::operator-=(const bigint &other)
{
    digits = (*this - other).digits;
    normalize();
    return *this;
}

bigint bigint::operator*(const bigint &other) const
{
    if (digits == "0" || other.digits == "0")
        return bigint(0);

    std::string a = digits;
    std::string b = other.digits;
    std::string result("0");
    for (int j = (int)b.size() - 1; j >= 0; j--)
    {
        std::string partial;
        int carry = 0;
        int digit = b[j] - '0';
        for (int i = (int)a.size() - 1; i >= 0; i--)
        {
            int prod = (a[i] - '0') * digit + carry;
            partial = char('0' + (prod % 10)) + partial;
            carry = prod / 10;
        }
        if (carry > 0)
            partial = char('0' + carry) + partial;
        for (int k = (int)b.size() - 1 - j; k > 0; k--)
            partial += '0';
        result = (bigint(result) + bigint(partial)).digits;
    }
    bigint r(result);
    r.normalize();
    return r;
}

bigint bigint::operator*(int n) const
{
    return *this * bigint(n);
}

bigint bigint::operator/(int n) const
{
    if (n == 0)
        return bigint(0);
    std::string result;
    int rem = 0;
    for (size_t i = 0; i < digits.size(); i++)
    {
        rem = rem * 10 + (digits[i] - '0');
        int q = rem / n;
        rem = rem % n;
        if (!result.empty() || q != 0)
            result += char('0' + q);
    }
    if (result.empty())
        result = "0";
    bigint r(result);
    r.normalize();
    return r;
}

bigint bigint::operator%(int n) const
{
    if (n == 0)
        return bigint(0);
    int rem = 0;
    for (size_t i = 0; i < digits.size(); i++)
        rem = (rem * 10 + (digits[i] - '0')) % n;
    return bigint(rem);
}

bigint bigint::operator<<(int n) const
{
    if (n <= 0)
        return *this;
    bigint r(digits + std::string(n, '0'));
    r.normalize();
    return r;
}

bigint bigint::operator>>(int n) const
{
    if (n <= 0)
        return *this;
    if ((size_t)n >= digits.size())
        return bigint(0);
    bigint r(digits.substr(0, digits.size() - (size_t)n));
    r.normalize();
    return r;
}

bigint &bigint::operator<<=(int n)
{
    digits = (*this << n).digits;
    normalize();
    return *this;
}

bigint &bigint::operator>>=(const bigint &n)
{
    digits = (*this >> str_to_int(n.digits)).digits;
    normalize();
    return *this;
}

bigint &bigint::operator++()
{
    *this = *this + bigint(1);
    return *this;
}

bigint bigint::operator++(int)
{
    bigint copy(*this);
    *this = *this + bigint(1);
    return copy;
}

bigint &bigint::operator--()
{
    if (digits == "0")
        return *this;
    *this = *this - bigint(1);
    return *this;
}

bigint bigint::operator--(int)
{
    bigint copy(*this);
    if (digits != "0")
        *this = *this - bigint(1);
    return copy;
}

bool bigint::operator<(const bigint &other) const
{
    if (digits.size() != other.digits.size())
        return digits.size() < other.digits.size();
    return digits < other.digits;
}

bool bigint::operator<=(const bigint &other) const
{
    return !(other < *this);
}

bool bigint::operator>(const bigint &other) const
{
    return other < *this;
}

bool bigint::operator>=(const bigint &other) const
{
    return !(*this < other);
}

bool bigint::operator==(const bigint &other) const
{
    return digits == other.digits;
}

bool bigint::operator!=(const bigint &other) const
{
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const bigint &b)
{
    os << b.digits;
    return os;
}
