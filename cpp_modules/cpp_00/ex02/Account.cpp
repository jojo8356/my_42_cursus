#include "Account.hpp"
#include <sstream>

Account::Account(void) : _balance(0)
{
    _timestamp = std::time(NULL);
    std::cout << "Default constructor called" << std::endl;
}

Account::Account(int initialDeposit) : _balance(initialDeposit)
{
    _timestamp = std::time(NULL);
    std::cout << "Account created on " << getDate()
              << " with initial deposit of " << _balance << std::endl;
}

Account::~Account(void)
{
    std::cout << "Destructor called" << std::endl;
}

void Account::deposit(int amount)
{
    if (amount <= 0)
    {
        log("Invalid deposit amount.");
        return;
    }
    _balance += amount;
    std::ostringstream oss;
    oss << "Amount deposited. New balance: " << _balance;
    log(oss.str());
}

void Account::withdraw(int amount)
{
    if (amount <= 0)
    {
        log("Invalid withdrawal amount.");
        return;
    }
    if (amount > _balance)
    {
        log("Not enough funds.");
        return;
    }
    _balance -= amount;
    std::ostringstream oss;
    oss << "Amount withdrawn. New balance: " << _balance;
    log(oss.str());
}

int Account::getBalance(void) const
{
    return _balance;
}

std::string Account::getDate(void) const
{
    char buffer[64];
    std::tm *tm = std::localtime(&_timestamp);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    return std::string(buffer);
}

void Account::log(const std::string &message) const
{
    std::cout << "[" << getDate() << "] " << message << std::endl;
}
