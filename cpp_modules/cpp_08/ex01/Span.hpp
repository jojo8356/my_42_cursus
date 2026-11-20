#ifndef SPAN_HPP
#define SPAN_HPP

#include <vector>
#include <algorithm>
#include <stdexcept>

class Span
{
public:
    Span(unsigned int N);
    Span(const Span &src);
    ~Span(void);

    Span &operator=(const Span &rhs);

    void addNumber(int n);
    unsigned int shortestSpan() const;
    unsigned int longestSpan() const;

    template <typename It>
    void addNumber(It begin, It end)
    {
        for (; begin != end; ++begin)
        {
            if (_v.size() >= _capacity)
                throw std::runtime_error("Span is full");
            _v.push_back(*begin);
        }
    }

    unsigned int getCapacity() const { return _capacity; }
    unsigned int getSize() const { return static_cast<unsigned int>(_v.size()); }

private:
    unsigned int _capacity;
    std::vector<int> _v;
};

#endif
