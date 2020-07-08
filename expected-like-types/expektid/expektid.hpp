#pragma warning(disable : 4068)
#pragma ide diagnostic ignored "google-explicit-constructor"

#pragma once

#include <variant>
#include <functional>

#define EXPEKTID_CONCAT_STR_(first_, second_) first_ ## second_
#define EXPEKTID_VAL_OR_RET_ERR(varName_, call_) auto EXPEKTID_CONCAT_STR_(tmp_, varName_) = call_; \
    if (!EXPEKTID_CONCAT_STR_(tmp_, varName_)) return EXPEKTID_CONCAT_STR_(tmp_, varName_).error(); \
    auto varName_ = EXPEKTID_CONCAT_STR_(tmp_, varName_).value();                                   \
    do{}while(false)

template <class ValueType, class ErrorType>
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
    Expektid(const ValueType& val) : result{val}
    {
    }

    Expektid(const ErrorType& err) : result{err}
    {
    }

    ~Expektid()
    {
        if (status == Status::Unused && std::uncaught_exceptions() == 0)
        {
            logAndTerminate("~Expektid(): Value not checked.");
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
        return hasValueInternal();
    }

    ValueType& operator*()
    {
        return value();
    }

    ValueType* operator->()
    {
        return &(value());
    }

    ValueType& value()
    {
        if (!hasValueInternal() || status != Status::Checked)
        {
            logAndTerminate("value(): Value not checked or is error.");
        }

        return valueInternal();
    }

    ValueType valueOr(const ValueType& orThis)
    {
        if (status != Status::Checked)
        {
            status = Status::Used;
        }

        if (hasValueInternal())
        {
            return valueInternal();
        }
        else
        {
            return orThis;
        }
    }

    template <typename CallableType>
    auto map(CallableType&& callable) -> Expektid<decltype(std::invoke(callable, std::declval<ValueType>())), ErrorType>
    {
        if (status != Status::Checked)
        {
            status = Status::Used;
        }

        if (hasValueInternal())
        {
            return std::invoke(callable, valueInternal());
        }
        else
        {
            return errorInternal();
        }
    }

    template <typename NewValueType>
    Expektid<NewValueType, ErrorType> mapButEasierToExplain(std::function<NewValueType(const ValueType&)> fun)
    {
        if (status != Status::Checked)
        {
            status = Status::Used;
        }

        if (hasValueInternal())
        {
            return fun(valueInternal());
        }
        else
        {
            return errorInternal();
        }
    }

    ErrorType& error()
    {
        if (hasValueInternal() || status != Status::Checked)
        {
            logAndTerminate("error(): Value not checked or is not error.");
        }

        return errorInternal();
    }

private:
    bool hasValueInternal() const
    {
        return (result.index() == 0);
    }

    ValueType& valueInternal()
    {
        return std::get<0>(result);
    }

    ErrorType& errorInternal()
    {
        return std::get<1>(result);
    }

    [[noreturn]] static void logAndTerminate(std::string_view str)
    {
        std::cerr << str << "\n";
        std::terminate();
    }

    std::variant<ValueType, ErrorType> result;
    Status status = Status::Unused;
};
