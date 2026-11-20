#include "Animal.hpp"
#include "Dog.hpp"
#include "Cat.hpp"

int main(void)
{
    const Animal *animals[10];
    for (int i = 0; i < 5; ++i)
        animals[i] = new Dog();
    for (int i = 5; i < 10; ++i)
        animals[i] = new Cat();

    for (int i = 0; i < 10; ++i)
    {
        std::cout << animals[i]->getType() << " ";
        animals[i]->makeSound();
    }

    {
        Dog original;
        original.getBrain()->setIdea(0, "I want a bone");
        Dog copy(original);
        copy.getBrain()->setIdea(0, "I want a fish");
        std::cout << "original idea: " << original.getBrain()->getIdea(0) << std::endl;
        std::cout << "copy idea: " << copy.getBrain()->getIdea(0) << std::endl;
    }

    for (int i = 0; i < 10; ++i)
        delete animals[i];
    return 0;
}
