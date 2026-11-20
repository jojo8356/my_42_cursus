#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#define BITCOIN_DATABASE "data.csv"
#define OLDEST_DATE      20090102
#define LATEST_YEAR      2023
#define ASSET_MIN        0
#define ASSET_MAX        1000

class BitcoinExchange
{
public:
    BitcoinExchange(void);
    ~BitcoinExchange(void);
    BitcoinExchange(const BitcoinExchange &src);
    BitcoinExchange &operator=(const BitcoinExchange &rhs);

    void showMonetaryValues(const std::string &input_file) const;

private:
    std::map<int, double> _btc_db;
    bool _valid_db;

    bool checkDate(int date) const;
    bool loadDataBase(void);
    void showBtcMonetaryValue(int date, double assets) const;

public:
    class InvalidFileException : public std::exception
    {
    public:
        virtual const char *what(void) const throw();
    };

    class NotPositiveNumberException : public std::exception
    {
    public:
        virtual const char *what(void) const throw();
    };

    class TooLargeNumberException : public std::exception
    {
    public:
        virtual const char *what(void) const throw();
    };
};

#endif
