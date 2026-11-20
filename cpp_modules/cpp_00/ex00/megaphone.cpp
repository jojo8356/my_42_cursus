#include <iostream>
#include <string>
#include <cctype>

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        std::cout << "* LOUD AND UNBEARABLE FEEDBACK NOISE *" << std::endl;
        return 0;
    }

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg(argv[i]);
        std::string upper;
        upper.reserve(arg.size());
        for (std::string::size_type j = 0; j < arg.size(); ++j)
            upper += static_cast<char>(std::toupper(static_cast<unsigned char>(arg[j])));
        std::cout << upper << " ";
    }
    std::cout << std::endl;
    return 0;
}
