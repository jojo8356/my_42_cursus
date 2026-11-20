#include "PmergeMe.hpp"
#include <cstdlib>
#include <algorithm>

static bool pairSecondLess(const std::pair<int, int> &x, const std::pair<int, int> &y)
{
    return x.second < y.second;
}

PmergeMe::PmergeMe(void)
{
}

PmergeMe::PmergeMe(int argc, char **argv)
{
    long min = 0;
    long max = 2147483647L;
    for (int i = 1; i < argc; ++i)
    {
        char *pEnd = NULL;
        long nb = std::strtol(argv[i], &pEnd, 10);
        if (*pEnd != '\0')
            throw NonDigitCharException();
        if (nb > max)
            throw std::runtime_error("Error: integer overflow.");
        if (nb < min)
            throw NegativeNumberException();
        _vec.push_back(static_cast<int>(nb));
        _lst.push_back(static_cast<int>(nb));
    }
}

PmergeMe::PmergeMe(const PmergeMe &src) : _vec(src._vec), _lst(src._lst)
{
}

PmergeMe::~PmergeMe(void)
{
}

PmergeMe &PmergeMe::operator=(const PmergeMe &rhs)
{
    if (this != &rhs)
    {
        _vec = rhs._vec;
        _lst = rhs._lst;
    }
    return *this;
}

std::vector<int> PmergeMe::mergeInsertVector(const std::vector<int> &input) const
{
    std::vector<std::pair<int, int> > pairs;
    std::vector<int>::const_iterator it = input.begin();
    while (it != input.end() && it + 1 != input.end())
    {
        int a = *it;
        int b = *(it + 1);
        if (a <= b)
            pairs.push_back(std::make_pair(a, b));
        else
            pairs.push_back(std::make_pair(b, a));
        it += 2;
    }

    std::sort(pairs.begin(), pairs.end(), pairSecondLess);

    std::vector<int> sorted;
    if (!pairs.empty())
    {
        sorted.push_back(pairs[0].first);
        sorted.push_back(pairs[0].second);
    }

    for (std::vector<int>::size_type i = 1; i < pairs.size(); ++i)
    {
        int b = pairs[i].second;
        int a = pairs[i].first;
        std::vector<int>::iterator pos = std::lower_bound(sorted.begin(), sorted.end(), b);
        std::vector<int>::iterator bpos = sorted.insert(pos, b);
        std::vector<int>::iterator posA = std::lower_bound(sorted.begin(), bpos + 1, a);
        sorted.insert(posA, a);
    }

    if (input.size() % 2 == 1)
    {
        int singleton = input.back();
        sorted.insert(std::lower_bound(sorted.begin(), sorted.end(), singleton), singleton);
    }
    return sorted;
}

std::list<int> PmergeMe::mergeInsertList(const std::list<int> &input) const
{
    std::list<std::pair<int, int> > pairs;
    std::list<int>::const_iterator it = input.begin();
    while (it != input.end())
    {
        std::list<int>::const_iterator nxt = it;
        ++nxt;
        if (nxt == input.end())
            break;
        int a = *it;
        int b = *nxt;
        if (a <= b)
            pairs.push_back(std::make_pair(a, b));
        else
            pairs.push_back(std::make_pair(b, a));
        std::advance(it, 2);
    }

    pairs.sort(pairSecondLess);

    std::list<int> sorted;
    if (!pairs.empty())
    {
        sorted.push_back(pairs.front().first);
        sorted.push_back(pairs.front().second);
    }

    std::list<std::pair<int, int> >::const_iterator pit = pairs.begin();
    ++pit;
    for (; pit != pairs.end(); ++pit)
    {
        int b = pit->second;
        int a = pit->first;
        std::list<int>::iterator posB = sorted.begin();
        while (posB != sorted.end() && *posB < b)
            ++posB;
        sorted.insert(posB, b);
        std::list<int>::iterator posA = sorted.begin();
        while (posA != posB && *posA < a)
            ++posA;
        sorted.insert(posA, a);
    }

    if (input.size() % 2 == 1)
    {
        int singleton = input.back();
        std::list<int>::iterator pos = sorted.begin();
        while (pos != sorted.end() && *pos < singleton)
            ++pos;
        sorted.insert(pos, singleton);
    }
    return sorted;
}

void PmergeMe::sortVector(void)
{
    _vec = mergeInsertVector(_vec);
}

void PmergeMe::sortList(void)
{
    _lst = mergeInsertList(_lst);
}

void PmergeMe::printVector(const std::string &label) const
{
    std::cout << label;
    for (std::vector<int>::const_iterator it = _vec.begin(); it != _vec.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
}

void PmergeMe::printList(const std::string &label) const
{
    std::cout << label;
    for (std::list<int>::const_iterator it = _lst.begin(); it != _lst.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
}

void PmergeMe::timedSort(void)
{
    size_t n = _vec.size();

    std::cout << "Before: ";
    for (std::vector<int>::const_iterator it = _vec.begin(); it != _vec.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;

    std::clock_t startV = std::clock();
    sortVector();
    std::clock_t stopV = std::clock();
    std::cout << "After: ";
    for (std::vector<int>::const_iterator it = _vec.begin(); it != _vec.end(); ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
    double timeV = static_cast<double>(stopV - startV) * 1000000.0 / CLOCKS_PER_SEC;
    std::cout << "Time to process a range of " << n
              << " elements with std::vector : " << timeV << " us" << std::endl;

    std::clock_t startL = std::clock();
    sortList();
    std::clock_t stopL = std::clock();
    double timeL = static_cast<double>(stopL - startL) * 1000000.0 / CLOCKS_PER_SEC;
    std::cout << "Time to process a range of " << n
              << " elements with std::list : " << timeL << " us" << std::endl;
}

const char *PmergeMe::NonDigitCharException::what(void) const throw()
{
    return "Error: arguments contain non-digit characters.";
}

const char *PmergeMe::NegativeNumberException::what(void) const throw()
{
    return "Error: negative number(s) not accepted.";
}
