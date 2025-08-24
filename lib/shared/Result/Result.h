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

    static Result<T> incomplete(T value) {
        return Result<T>(value, "__INCOMPLETE_PACKET__");
    }

    static Result<T> failure(const std::string &errorMessage) {
        return Result<T>(std::nullopt, errorMessage);
    }

    static Result<T> fromOptional(std::optional<T> value, const std::string &errorMessage) {
        if (value.has_value()) {
            return success(value.value());
        }
        return failure(errorMessage);
    }

    [[nodiscard]] bool isSuccess() const { return errorMessage.empty(); }

    [[nodiscard]] bool isError() const { return !isSuccess(); }

    [[nodiscard]] bool isIncomplete() const { return errorMessage == "__INCOMPLETE_PACKET__"; }

    [[nodiscard]] const T &getValue() const { return value.value(); }

    [[nodiscard]] const std::string &getError() const { return errorMessage; }

private:
    std::optional<T> value;
    std::string errorMessage;

    Result(std::optional<T> value, std::string errorMessage)
        : value(std::move(value)), errorMessage(std::move(errorMessage)) {
    }
};

#endif // RESULT_H
