#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <vector>
#include <list>
#include <utility>
#include <string>
#include <iostream>
#include <stdexcept>
#include <ctime>

class PmergeMe
{
public:
    PmergeMe(int argc, char **argv);
    ~PmergeMe(void);
    PmergeMe(const PmergeMe &src);
    PmergeMe &operator=(const PmergeMe &rhs);

    void timedSort(void);

private:
    PmergeMe(void);
    std::vector<int> _vec;
    std::list<int> _lst;

    void sortVector(void);
    void sortList(void);
    std::vector<int> mergeInsertVector(const std::vector<int> &input) const;
    std::list<int> mergeInsertList(const std::list<int> &input) const;

    void printVector(const std::string &label) const;
    void printList(const std::string &label) const;

public:
    class NonDigitCharException : public std::exception
    {
    public:
        virtual const char *what(void) const throw();
    };

    class NegativeNumberException : public std::exception
    {
    public:
        virtual const char *what(void) const throw();
    };
};

#endif
