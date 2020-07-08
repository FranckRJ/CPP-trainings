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

Expektid<std::string, Error> callDoStuff(bool shouldSuccess)
{
    EXPEKTID_VAL_OR_RET_ERR(baseRes, doStuff(shouldSuccess));
    EXPEKTID_VAL_OR_RET_ERR(combinedRes, doStuff(!baseRes.empty()));
    return combinedRes;
}

int main()
{
    try
    {
        auto expected = callDoStuff(true);

        std::cout << "Func maybe returned: " << expected.valueOr("default value") << '\n';

        if (expected)
        {
            std::cout << "Func succeed with result: " << expected.value() << '\n';
        }
        else
        {
            std::cout << "Func failed with error: " << expected.error().message << "\n";
        }

        auto size = expected.mapButEasierToExplain<std::size_t>(&std::string::size);
        std::cout << "Size of str is maybe: " << size.valueOr(153) << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shouldn't happen: " << e.what() << '\n';
    }
    return 0;
}
