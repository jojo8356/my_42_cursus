#include "PhoneBook.hpp"

PhoneBook::PhoneBook(void) : _count(0)
{
}

PhoneBook::~PhoneBook(void)
{
}

void PhoneBook::addContact(const Contact &contact)
{
    std::size_t index = _count;
    if (_count >= _maxContacts)
        index = 0;
    _contacts[index] = contact;
    if (_count < _maxContacts)
        ++_count;
}

const Contact &PhoneBook::getContact(std::size_t index) const
{
    return _contacts[index];
}

std::size_t PhoneBook::getCount(void) const
{
    return _count;
}

std::string PhoneBook::truncate(const std::string &text) const
{
    if (text.size() <= 10)
        return text;
    return text.substr(0, 9) + ".";
}
