#include "Contact.hpp"

Contact::Contact(void)
{
}

Contact::~Contact(void)
{
}

void Contact::setFirstName(const std::string &firstName)
{
    _firstName = firstName;
}

void Contact::setLastName(const std::string &lastName)
{
    _lastName = lastName;
}

void Contact::setNickname(const std::string &nickname)
{
    _nickname = nickname;
}

void Contact::setPhoneNumber(const std::string &phoneNumber)
{
    _phoneNumber = phoneNumber;
}

void Contact::setDarkestSecret(const std::string &darkestSecret)
{
    _darkestSecret = darkestSecret;
}

const std::string &Contact::getFirstName(void) const
{
    return _firstName;
}

const std::string &Contact::getLastName(void) const
{
    return _lastName;
}

const std::string &Contact::getNickname(void) const
{
    return _nickname;
}

const std::string &Contact::getPhoneNumber(void) const
{
    return _phoneNumber;
}

const std::string &Contact::getDarkestSecret(void) const
{
    return _darkestSecret;
}

bool Contact::isValid(void) const
{
    return !_firstName.empty() && !_lastName.empty() && !_nickname.empty()
        && !_phoneNumber.empty() && !_darkestSecret.empty();
}
