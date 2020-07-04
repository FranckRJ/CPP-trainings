#pragma warning(disable : 4068)
#pragma ide diagnostic ignored "google-explicit-constructor"

#pragma once

#include <stdexcept>
#include <variant>

template <class SuccessType, class FailureType>
class [[nodiscard]] Expektid
{
public:
    Expektid(const SuccessType& success) : result{success}
    {
    }

    Expektid(const FailureType& failure) : result{failure}
    {
    }

    ~Expektid() noexcept(false)
    {
        if (!checked)
        {
            checked = true;
            throw std::runtime_error{"Value not checked."};
        }
    }

    Expektid(const Expektid&) = delete;

    Expektid(Expektid&& other) noexcept : result{std::move(other.result)}, checked{std::exchange(other.checked, true)}
    {
    }

    Expektid& operator=(const Expektid&) = delete;

    Expektid& operator=(Expektid&& other) noexcept
    {
        result = std::move(other.result);
        checked = std::exchange(other.checked, true);
        return *this;
    }

    operator bool()
    {
        checked = true;
        return resultIsSuccess();
    }

    SuccessType& operator*()
    {
        return getValue();
    }

    SuccessType* operator->()
    {
        return &(getValue());
    }

    SuccessType& getValue()
    {
        if (!resultIsSuccess() || !checked)
        {
            checked = true;
            throw std::runtime_error{"Value not checked or not success."};
        }

        return std::get<0>(result);
    }

    SuccessType getValueOr(SuccessType orThis) const
    {
        if (resultIsSuccess())
        {
            return std::get<0>(result);
        }
        else
        {
            return orThis;
        }
    }

    FailureType& getError()
    {
        if (resultIsSuccess() || !checked)
        {
            checked = true;
            throw std::runtime_error{"Value not checked or not failed."};
        }

        return std::get<1>(result);
    }

private:
    [[nodiscard]] bool resultIsSuccess() const
    {
        return (result.index() == 0);
    }

    std::variant<SuccessType, FailureType> result;
    bool checked = false;
};
