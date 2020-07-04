#include <iostream>

#include "expektid.hpp"

struct Error
{
    std::string message;
};

Expektid<std::string, Error> doStuff(bool shouldSuccess)
{
    if (shouldSuccess)
    {
        return std::string{"all good !"};
    }
    else
    {
        return Error{"not good"};
    }
}

int main()
{
    try
    {
        auto expected = doStuff(true);

        std::cout << "Func maybe returned: " << expected.getValueOr("default value") << '\n';

        if (expected)
        {
            std::cout << "Func succeed with result: " << expected.getValue() << '\n';
        }
        else
        {
            std::cout << "Func failed with error: " << expected.getError().message << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shouldn't happen unless there is a bug in the program: " << e.what() << '\n';
    }
    return 0;
}
