#ifndef ANIMAL_HPP
#define ANIMAL_HPP

#include <iostream>
#include <string>

class Animal
{
    public:
        Animal(void);
        Animal(const std::string &type);
        Animal(const Animal &src);

        Animal &operator=(const Animal &rhs);

        virtual ~Animal(void);
        virtual void makeSound(void) const;

        std::string getType(void) const;

    protected:
        std::string _type;
};

#endif
