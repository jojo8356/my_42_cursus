#ifndef PHONEBOOK_HPP
#define PHONEBOOK_HPP

#include "Contact.hpp"

class PhoneBook
{
    public:
        PhoneBook(void);
        ~PhoneBook(void);

        void addContact(const Contact &contact);
        const Contact &getContact(std::size_t index) const;
        std::size_t getCount(void) const;

        std::string truncate(const std::string &text) const;

    private:
        static const std::size_t _maxContacts = 8;
        Contact _contacts[8];
        std::size_t _count;
};

#endif
