#include <iostream>
#include <fstream>
#include <string>

static std::string replaceAll(const std::string &content,
                              const std::string &from,
                              const std::string &to)
{
    std::string result;
    std::string::size_type pos = 0;
    while (pos < content.size())
    {
        std::string::size_type found = content.find(from, pos);
        if (found == std::string::npos)
        {
            result += content.substr(pos);
            break;
        }
        result += content.substr(pos, found - pos);
        result += to;
        pos = found + from.size();
    }
    return result;
}

static std::string readFile(const std::string &path)
{
    std::ifstream input(path.c_str());
    if (!input)
        return std::string();
    std::string content;
    std::string line;
    while (std::getline(input, line))
        content += line + "\n";
    return content;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <filename> <s1> <s2>" << std::endl;
        return 1;
    }

    const std::string filename = argv[1];
    const std::string s1 = argv[2];
    const std::string s2 = argv[3];

    std::string content = readFile(filename);
    if (content.empty())
    {
        std::cerr << "Error: could not read file '" << filename << "'." << std::endl;
        return 1;
    }

    std::string result = replaceAll(content, s1, s2);

    std::string outName = filename + ".replace";
    std::ofstream output(outName.c_str());
    if (!output)
    {
        std::cerr << "Error: could not write file '" << outName << "'." << std::endl;
        return 1;
    }
    output << result;
    return 0;
}
