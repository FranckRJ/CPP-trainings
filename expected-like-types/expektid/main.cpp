#include <iostream>

#include "expektid.hpp"

struct Success
{
    int value;
};

struct Error
{
    std::string message;
};

Expektid<Success, Error> doStuff(bool shouldSuccess)
{
    if (shouldSuccess)
    {
        return Success{5};
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

        if (expected)
        {
            std::cout << "Func succeed with result: " << expected->value << '\n';
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
