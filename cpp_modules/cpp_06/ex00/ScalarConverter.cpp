#include "ScalarConverter.hpp"
#include <cstdlib>
#include <cmath>
#include <cctype>

enum Type
{
    CHAR,
    INT,
    FLOAT,
    DOUBLE
};

static Type detect(const std::string &s)
{
    if (s == "inf" || s == "+inf" || s == "-inf" || s == "nan")
        return DOUBLE;
    if (s.size() == 1)
    {
        if (std::isdigit(static_cast<unsigned char>(s[0])))
            return INT;
        return CHAR;
    }
    bool hasF = false;
    for (std::string::size_type i = 0; i < s.size(); ++i)
    {
        if (s[i] == 'f' || s[i] == 'F')
            hasF = true;
    }
    bool hasDotOrExp = false;
    for (std::string::size_type i = 0; i < s.size(); ++i)
    {
        if (s[i] == '.' || s[i] == 'e' || s[i] == 'E')
            hasDotOrExp = true;
    }
    if (hasF)
        return FLOAT;
    if (hasDotOrExp)
        return DOUBLE;
    return INT;
}

void ScalarConverter::convert(const std::string &s)
{
    Type t = detect(s);
    double dval = 0.0;
    bool special = false;
    std::string specialVal;

    if (t == CHAR)
        dval = static_cast<double>(static_cast<unsigned char>(s[0]));
    else if (t == INT)
        dval = static_cast<double>(std::strtol(s.c_str(), NULL, 10));
    else if (t == FLOAT || t == DOUBLE)
    {
        std::string noF = s;
        if (noF == "nan" || noF == "inf" || noF == "+inf" || noF == "-inf")
        {
            special = true;
            specialVal = noF;
        }
        if (!noF.empty())
        {
            char last = noF[noF.size() - 1];
            if (last == 'f' || last == 'F')
                noF = noF.substr(0, noF.size() - 1);
        }
        dval = std::strtod(noF.c_str(), NULL);
    }

    if (special)
        std::cout << "char: impossible" << std::endl;
    else
    {
        char c = static_cast<char>(dval);
        if (std::isprint(static_cast<unsigned char>(c)))
            std::cout << "char: '" << c << "'" << std::endl;
        else
            std::cout << "char: Non displayable" << std::endl;
    }

    if (special)
        std::cout << "int: impossible" << std::endl;
    else
        std::cout << "int: " << static_cast<long>(dval) << std::endl;

    if (special)
    {
        if (t == FLOAT)
        {
            if (specialVal == "nan")
                std::cout << "float: nanf" << std::endl;
            else if (specialVal == "inf" || specialVal == "+inf")
                std::cout << "float: inff" << std::endl;
            else
                std::cout << "float: -inff" << std::endl;
        }
        else
            std::cout << "float: impossible" << std::endl;
    }
    else
        std::cout << "float: " << static_cast<float>(dval) << "f" << std::endl;

    if (special)
    {
        if (specialVal == "nan")
            std::cout << "double: nan" << std::endl;
        else if (specialVal == "inf" || specialVal == "+inf")
            std::cout << "double: inf" << std::endl;
        else
            std::cout << "double: -inf" << std::endl;
    }
    else
        std::cout << "double: " << dval << std::endl;
}
