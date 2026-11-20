#ifndef WRONGANIMAL_HPP
#define WRONGANIMAL_HPP

#include <iostream>
#include <string>

class WrongAnimal
{
    public:
        WrongAnimal(void);
        WrongAnimal(const std::string &type);
        WrongAnimal(const WrongAnimal &src);
        ~WrongAnimal(void);

        WrongAnimal &operator=(const WrongAnimal &rhs);

        void makeSound(void) const;
        std::string getType(void) const;

    protected:
        std::string _type;
};

#endif
