// Result.h
#ifndef RESULT_H
#define RESULT_H

#pragma message ("Compiling Result.h from: " __FILE__)

#include <optional>
#include <string>

template <typename T>
class Result
{
public:
    static Result<T> success(T value);
    static Result<T> pending(const std::string& errorMessage);
    static Result<T> failure(const std::string& errorMessage);
    static Result<T> fromOptional(std::optional<T> value, const std::string& errorMessage);

    [[nodiscard]] bool isSuccess() const { return type == Type::Success; }
    [[nodiscard]] bool isError() const { return type == Type::Failure; }
    [[nodiscard]] bool isPending() const { return type == Type::Incomplete; }

    [[nodiscard]] const T& getValueConst() const { return value.value(); }
    [[nodiscard]] T& getValue() { return value.value(); }
    [[nodiscard]] const std::string& getError() const { return errorMessage; }

private:

    enum class Type { Success, Incomplete, Failure };
    std::optional<T> value;
    std::string errorMessage;
    Type type;

    Result(std::optional<T> value, std::string errorMessage, Type type);

    // Result(std::string errorMessage, const Type type) : errorMessage(std::move(errorMessage)), type(type) {}
    // Result(std::string errorMessage, const Type type);
};

template <>
class Result<void>
{
public:
    static Result<void> success();
    static Result<void> pending(const std::string& errorMessage);
    static Result<void> failure(const std::string& errorMessage);

    [[nodiscard]] bool isSuccess() const { return type == Type::Success; }
    [[nodiscard]] bool isError() const { return type == Type::Failure; }
    [[nodiscard]] bool isPending() const { return type == Type::Incomplete; }
    [[nodiscard]] const std::string& getError() const { return errorMessage; }

private:
    enum class Type { Success, Incomplete, Failure };
    std::string errorMessage;
    Type type;

    Result(std::string errorMessage, const Type type);
};

#endif // RESULT_H
