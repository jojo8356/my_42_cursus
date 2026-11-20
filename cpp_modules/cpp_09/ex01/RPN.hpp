#ifndef RPN_HPP
#define RPN_HPP

#include <deque>
#include <string>
#include <iostream>
#include <stdexcept>

class RPN
{
public:
    RPN(const std::string &input);
    ~RPN(void);
    RPN(const RPN &src);
    RPN &operator=(const RPN &rhs);

    void calculator(void);

private:
    RPN(void);
    std::deque<int> _stack;
    std::string _input;

    void applyOperator(char op);
};

#endif
