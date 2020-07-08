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
        auto expected = doStuff(false);

        std::cout << "Func maybe returned: " << expected.valueOr("default value") << '\n';

        if (expected)
        {
            std::cout << "Func succeed with result: " << expected.value() << '\n';
        }
        else
        {
            std::cout << "Func failed with error: " << expected.error().message << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shouldn't happen: " << e.what() << '\n';
    }
    return 0;
}
