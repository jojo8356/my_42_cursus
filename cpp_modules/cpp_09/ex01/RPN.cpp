#include "RPN.hpp"

RPN::RPN(const std::string &input) : _input(input)
{
}

RPN::RPN(const RPN &src) : _stack(src._stack), _input(src._input)
{
}

RPN::~RPN(void)
{
}

RPN &RPN::operator=(const RPN &rhs)
{
    if (this != &rhs)
    {
        _stack = rhs._stack;
        _input = rhs._input;
    }
    return *this;
}

void RPN::applyOperator(char op)
{
    if (_stack.size() < 2)
        throw std::runtime_error("Error");
    int a = _stack.back();
    _stack.pop_back();
    int b = _stack.back();
    _stack.pop_back();
    switch (op)
    {
    case '+':
        _stack.push_back(b + a);
        break;
    case '-':
        _stack.push_back(b - a);
        break;
    case '*':
        _stack.push_back(b * a);
        break;
    case '/':
        if (a == 0)
            throw std::runtime_error("Error");
        _stack.push_back(b / a);
        break;
    default:
        throw std::runtime_error("Error");
    }
}

void RPN::calculator(void)
{
    bool canPush = true;
    for (std::string::size_type i = 0; i < _input.size(); ++i)
    {
        char c = _input[i];
        if (c == ' ')
        {
            canPush = true;
        }
        else if (c >= '0' && c <= '9')
        {
            if (!canPush)
                throw std::runtime_error("Error");
            _stack.push_back(c - '0');
            canPush = false;
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/')
        {
            applyOperator(c);
            canPush = false;
        }
        else
        {
            throw std::runtime_error("Error");
        }
    }
    if (_stack.size() != 1)
        throw std::runtime_error("Error");
    std::cout << _stack.back() << std::endl;
}
