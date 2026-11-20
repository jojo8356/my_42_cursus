#include "Span.hpp"

Span::Span(unsigned int N) : _capacity(N)
{
}

Span::Span(const Span &src) : _capacity(src._capacity), _v(src._v)
{
}

Span::~Span(void)
{
}

Span &Span::operator=(const Span &rhs)
{
    if (this != &rhs)
    {
        _capacity = rhs._capacity;
        _v = rhs._v;
    }
    return *this;
}

void Span::addNumber(int n)
{
    if (_v.size() >= _capacity)
        throw std::runtime_error("Span is full");
    _v.push_back(n);
}

unsigned int Span::shortestSpan() const
{
    if (_v.size() < 2)
        throw std::runtime_error("Not enough elements");
    std::vector<int> tmp = _v;
    std::sort(tmp.begin(), tmp.end());
    unsigned int min = static_cast<unsigned int>(tmp[1] - tmp[0]);
    for (std::vector<int>::size_type i = 1; i + 1 < tmp.size(); ++i)
    {
        unsigned int diff = static_cast<unsigned int>(tmp[i + 1] - tmp[i]);
        if (diff < min)
            min = diff;
    }
    return min;
}

unsigned int Span::longestSpan() const
{
    if (_v.size() < 2)
        throw std::runtime_error("Not enough elements");
    std::vector<int> tmp = _v;
    std::sort(tmp.begin(), tmp.end());
    return static_cast<unsigned int>(tmp.back() - tmp.front());
}
