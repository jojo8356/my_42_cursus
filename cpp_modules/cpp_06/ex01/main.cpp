#include <stdint.h>
#include "Serializer.hpp"
#include <iostream>

int main(void)
{
    Data original;
    original.id = 42;
    original.value = 3.14;
    original.name = "test";

    uintptr_t raw = Serializer::serialize(&original);
    Data *deserialized = Serializer::deserialize(raw);

    std::cout << "original addr: " << &original << std::endl;
    std::cout << "deserialized addr: " << deserialized << std::endl;
    std::cout << "equal: " << (deserialized == &original ? "true" : "false") << std::endl;
    std::cout << "id: " << deserialized->id
              << ", value: " << deserialized->value
              << ", name: " << deserialized->name << std::endl;
    return 0;
}
