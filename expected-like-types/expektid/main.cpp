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
            std::cout << "Return value is: " << expected->value << '\n';
        }
        else
        {
            std::cout << "Error in func\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
