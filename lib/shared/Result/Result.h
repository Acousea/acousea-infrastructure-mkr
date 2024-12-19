#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <optional>

template<typename T>
class Result {
public:
    static Result<T> success(T value) {
        return Result<T>(value, "");
    }

    static Result<T> failure(const std::string &errorMessage) {
        return Result<T>(std::nullopt, errorMessage);
    }

    [[nodiscard]] bool isSuccess() const { return errorMessage.empty(); }

    [[nodiscard]] bool isError() const { return !isSuccess(); }

    const T &getValue() const { return value.value(); }

    [[nodiscard]] const std::string &getError() const { return errorMessage; }

private:
    std::optional<T> value;
    std::string errorMessage;

    Result(std::optional<T> value, std::string errorMessage)
            : value(std::move(value)), errorMessage(std::move(errorMessage)) {}
};

#endif // RESULT_H