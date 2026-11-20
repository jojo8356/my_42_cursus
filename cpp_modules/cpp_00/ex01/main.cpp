#include "PhoneBook.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

static bool readLine(std::string &line)
{
    if (!std::getline(std::cin, line))
        return false;
    return true;
}

static void displayContact(const Contact &c)
{
    std::cout << "First name : " << c.getFirstName() << std::endl;
    std::cout << "Last name : " << c.getLastName() << std::endl;
    std::cout << "Nickname : " << c.getNickname() << std::endl;
    std::cout << "Phone number : " << c.getPhoneNumber() << std::endl;
    std::cout << "Darkest secret : " << c.getDarkestSecret() << std::endl;
}

int main(void)
{
    PhoneBook phonebook;
    std::string command;

    while (true)
    {
        std::cout << "Enter command (ADD, SEARCH, EXIT): ";
        if (!readLine(command))
            break;

        if (command == "ADD")
        {
            Contact contact;
            std::string first, last, nickname, phone, secret;
            if (!readLine(first) || !readLine(last) || !readLine(nickname)
                || !readLine(phone) || !readLine(secret))
                break;

            contact.setFirstName(first);
            contact.setLastName(last);
            contact.setNickname(nickname);
            contact.setPhoneNumber(phone);
            contact.setDarkestSecret(secret);

            if (contact.isValid())
                phonebook.addContact(contact);
            else
                std::cout << "Invalid contact: all fields must be non-empty." << std::endl;
        }
        else if (command == "SEARCH")
        {
            std::size_t count = phonebook.getCount();
            std::cout << std::setw(10) << "index" << "|"
                      << std::setw(10) << "first name" << "|"
                      << std::setw(10) << "last name" << "|"
                      << std::setw(10) << "nickname" << std::endl;
            for (std::size_t i = 0; i < count; ++i)
            {
                const Contact &c = phonebook.getContact(i);
                std::cout << std::setw(10) << i << "|"
                          << std::setw(10) << phonebook.truncate(c.getFirstName()) << "|"
                          << std::setw(10) << phonebook.truncate(c.getLastName()) << "|"
                          << std::setw(10) << phonebook.truncate(c.getNickname()) << std::endl;
            }
            if (count == 0)
            {
                std::cout << "No contacts to display." << std::endl;
                continue;
            }
            std::cout << "Enter an index: ";
            std::string indexStr;
            if (!readLine(indexStr))
                break;
            if (indexStr.empty())
            {
                std::cout << "Invalid index." << std::endl;
                continue;
            }
            char *end = NULL;
            long index = std::strtol(indexStr.c_str(), &end, 10);
            if (end == indexStr.c_str() || *end != '\0' || index < 0
                || static_cast<std::size_t>(index) >= count)
            {
                std::cout << "Invalid index." << std::endl;
                continue;
            }
            displayContact(phonebook.getContact(static_cast<std::size_t>(index)));
        }
        else if (command == "EXIT")
        {
            break;
        }
        else
        {
            std::cout << "Unknown command. Please use ADD, SEARCH or EXIT." << std::endl;
        }
    }
    return 0;
}
