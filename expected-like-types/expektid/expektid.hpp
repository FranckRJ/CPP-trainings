#pragma warning(disable : 4068)
#pragma ide diagnostic ignored "google-explicit-constructor"

#pragma once

#include <stdexcept>
#include <variant>

template <class SuccessType, class FailureType>
class [[nodiscard]] Expektid
{
private:
    enum class Status : char
    {
        Unused,
        Used,
        Checked
    };
public:
    Expektid(const SuccessType& success) : result{success}
    {
    }

    Expektid(const FailureType& failure) : result{failure}
    {
    }

    ~Expektid() noexcept(false)
    {
        if (status == Status::Unused && std::uncaught_exceptions() == 0)
        {
            throw std::runtime_error{"Value not checked."};
        }
    }

    Expektid(const Expektid&) = delete;

    Expektid(Expektid&& other) noexcept
        : result{std::move(other.result)}, status{std::exchange(other.status, Status::Used)}
    {
    }

    Expektid& operator=(const Expektid&) = delete;

    Expektid& operator=(Expektid&& other) noexcept
    {
        result = std::move(other.result);
        status = std::exchange(other.status, Status::Used);
        return *this;
    }

    operator bool()
    {
        status = Status::Checked;
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
        if (!resultIsSuccess() || status != Status::Checked)
        {
            throw std::runtime_error{"Value not checked or not success."};
        }

        return std::get<0>(result);
    }

    SuccessType getValueOr(SuccessType orThis)
    {
        if (status != Status::Checked)
        {
            status = Status::Used;
        }

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
        if (resultIsSuccess() || status != Status::Checked)
        {
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
    Status status = Status::Unused;
};
