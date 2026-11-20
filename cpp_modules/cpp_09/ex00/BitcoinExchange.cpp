#include "BitcoinExchange.hpp"

BitcoinExchange::BitcoinExchange(void) : _valid_db(false)
{
    _valid_db = loadDataBase();
}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &src) : _btc_db(src._btc_db), _valid_db(src._valid_db)
{
}

BitcoinExchange::~BitcoinExchange(void)
{
}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &rhs)
{
    if (this != &rhs)
    {
        _btc_db = rhs._btc_db;
        _valid_db = rhs._valid_db;
    }
    return *this;
}

bool BitcoinExchange::checkDate(int date) const
{
    int yy = date / 10000;
    int mm = (date - yy * 10000) / 100;
    int dd = date - yy * 10000 - mm * 100;

    if (yy < 2009 || yy > LATEST_YEAR || mm < 1 || mm > 12 || dd < 1 || dd > 31)
        return false;
    if (mm == 2 && ((yy % 4 != 0 && dd > 28) || (yy % 4 == 0 && dd > 29)))
        return false;
    if ((mm == 4 || mm == 6 || mm == 9 || mm == 11) && dd == 31)
        return false;
    return true;
}

bool BitcoinExchange::loadDataBase(void)
{
    std::ifstream ifs;
    std::string line;

    try
    {
        ifs.open(BITCOIN_DATABASE);
        if (!ifs.is_open())
            throw std::runtime_error("Error: could not open data file.");
        std::getline(ifs, line);
        if (line != "date,exchange_rate")
            throw std::runtime_error("Error: invalid data file.");
        while (std::getline(ifs, line))
        {
            std::istringstream iss(line);
            std::string dateStr, valueStr;
            if (!std::getline(iss, dateStr, ',') || !std::getline(iss, valueStr))
                throw std::runtime_error("Error: invalid data format");
            int yy, mm, dd;
            double value;
            if (sscanf(dateStr.c_str(), "%4d-%2d-%2d", &yy, &mm, &dd) != 3)
                throw std::runtime_error("Error: invalid data format");
            std::istringstream vs(valueStr);
            if (!(vs >> value))
                throw std::runtime_error("Error: invalid data format");
            int date = yy * 10000 + mm * 100 + dd;
            if (!checkDate(date))
                throw std::runtime_error("Error: date out of range");
            _btc_db[date] = value;
        }
        ifs.close();
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return false;
}

void BitcoinExchange::showBtcMonetaryValue(int date, double assets) const
{
    int key = date;
    std::map<int, double>::const_iterator it;

    while (key >= OLDEST_DATE)
    {
        it = _btc_db.find(key);
        if (it != _btc_db.end())
        {
            double result = assets * it->second;
            std::cout.precision(2);
            std::cout << std::fixed << result << std::endl;
            std::cout.unsetf(std::ios::fixed);
            return;
        }
        if ((key % 100) == 1)
        {
            if ((key % 10000) == 101)
                key -= 8870;
            else
                key -= 70;
        }
        else
            key--;
    }
}

void BitcoinExchange::showMonetaryValues(const std::string &input_file) const
{
    std::ifstream ifs(input_file.c_str());
    std::string line;

    try
    {
        if (!_valid_db)
            return;
        if (!ifs.is_open())
            throw InvalidFileException();
        std::getline(ifs, line);
        if (line != "date | value")
            throw std::runtime_error("Error: invalid input file. Header missing.");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    while (std::getline(ifs, line))
    {
        try
        {
            std::string::size_type pos = line.find(" | ");
            if (pos == std::string::npos)
                throw std::runtime_error("Error: bad input => " + line);
            std::string dateStr = line.substr(0, pos);
            std::string valueStr = line.substr(pos + 3);

            int yy, mm, dd;
            if (sscanf(dateStr.c_str(), "%4d-%2d-%2d", &yy, &mm, &dd) != 3)
                throw std::runtime_error("Error: bad input => " + line);
            int date = yy * 10000 + mm * 100 + dd;
            if (date < OLDEST_DATE)
                throw std::runtime_error("Error: bad input => " + line);
            if (!checkDate(date))
                throw std::runtime_error("Error: bad input => " + line);

            std::istringstream vs(valueStr);
            double assets;
            if (!(vs >> assets))
                throw std::runtime_error("Error: bad input => " + line);
            if (assets < ASSET_MIN)
                throw NotPositiveNumberException();
            if (assets > ASSET_MAX)
                throw TooLargeNumberException();

            std::cout << line << " => ";
            showBtcMonetaryValue(date, assets);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    ifs.close();
}

const char *BitcoinExchange::InvalidFileException::what(void) const throw()
{
    return "Error: could not open file.";
}

const char *BitcoinExchange::NotPositiveNumberException::what(void) const throw()
{
    return "Error: not a positive number.";
}

const char *BitcoinExchange::TooLargeNumberException::what(void) const throw()
{
    return "Error: too large a number.";
}
