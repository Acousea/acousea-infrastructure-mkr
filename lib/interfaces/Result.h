#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <optional>

template <typename T>
class Result
{
public:
    static Result<T> success(T value)
    {
        return Result<T>(value, "", Type::Success);
    }

    static Result<T> pending(const std::string& errorMessage)
    {
        return Result<T>(std::nullopt, errorMessage, Type::Incomplete);
    }

    static Result<T> failure(const std::string& errorMessage)
    {
        return Result<T>(std::nullopt, errorMessage, Type::Failure);
    }

    static Result<T> fromOptional(std::optional<T> value, const std::string& errorMessage)
    {
        if (value.has_value())
        {
            return success(value.value());
        }
        return failure(errorMessage);
    }

    [[nodiscard]] bool isSuccess() const { return type == Type::Success; }

    [[nodiscard]] bool isError() const { return type == Type::Failure; }

    [[nodiscard]] bool isPending() const { return type == Type::Incomplete; }

    [[nodiscard]] const T& getValueConst() const { return value.value(); }

    [[nodiscard]] T& getValue() { return value.value(); }

    [[nodiscard]] const std::string& getError() const { return errorMessage; }

private:
    enum class Type
    {
        Success,
        Incomplete,
        Failure
    };

    std::optional<T> value;
    std::string errorMessage;
    Result::Type type;


    Result(std::optional<T> value, std::string errorMessage, const Type type) : value(std::move(value)),
                                                                          errorMessage(std::move(errorMessage)),
                                                                          type(type)
    {
    }
};

template <>
class Result<void>
{
public:
    static Result<void> success()
    {
        return Result<void>("", Type::Success);
    }

    static Result<void> pending(const std::string& errorMessage)
    {
        return Result<void>(errorMessage, Type::Incomplete);
    }

    static Result<void> failure(const std::string& errorMessage)
    {
        return Result<void>(errorMessage, Type::Failure);
    }

    [[nodiscard]] bool isSuccess() const { return type == Type::Success; }
    [[nodiscard]] bool isError() const { return type == Type::Failure; }
    [[nodiscard]] bool isPending() const { return type == Type::Incomplete; }

    [[nodiscard]] const std::string& getError() const { return errorMessage; }

private:
    enum class Type
    {
        Success,
        Incomplete,
        Failure
    };

    std::string errorMessage;
    Type type;

    Result(std::string errorMessage, const Type type)
        : errorMessage(std::move(errorMessage)), type(type) {}
};

#endif // RESULT_H
