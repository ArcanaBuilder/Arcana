#include "Util.h"


Arguments ParseArgs(int argc, char** argv)
{
    Arguments args;

    for (uint32_t i = 0; i < (uint32_t) argc; ++i)
    {
        args.push_back( Argument{ i, argv[i] } );
    }

    return args;
}


std::string ltrim(const std::string& s)
{
    const std::string ws = " \t\r\n\f\v";

    size_t pos = s.find_first_not_of(ws);
    if (pos == std::string::npos)
    {
        return "";
    }

    return s.substr(pos);
}


void ltrim(std::string& s)
{
    const std::string ws = " \t\r\n\f\v";
    size_t pos = s.find_first_not_of(ws);

    if (pos == std::string::npos)
    {
        s.clear();
    }
    else
    {
        s.erase(0, pos);
    }
}