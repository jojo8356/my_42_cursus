#include "Account.hpp"

int main(void)
{
    {
        Account a;
        a.deposit(100);
        a.withdraw(40);
        a.withdraw(200);
        a.deposit(-5);
    }
    {
        Account b(500);
        b.withdraw(250);
        b.deposit(10);
    }
    return 0;
}
