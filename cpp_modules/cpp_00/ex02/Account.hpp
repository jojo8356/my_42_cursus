#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include <iostream>
#include <string>
#include <ctime>

class Account
{
    public:
        Account(void);
        Account(int initialDeposit);
        ~Account(void);

        void deposit(int amount);
        void withdraw(int amount);

        int getBalance(void) const;

    private:
        int _balance;
        std::time_t _timestamp;

        std::string getDate(void) const;
        void log(const std::string &message) const;
};

#endif
