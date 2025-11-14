// Result.h
#ifndef RESULT_H
#define RESULT_H

// #pragma message ("Compiling Result.h from: " __FILE__)

#include <optional>

template <typename T>
class Result
{
public:
    enum class Type { Success = 0, Incomplete = 1, Failure = 2 };

    static Result<T> success(T value);
    static Result<T> pending(const char* errorMessage);
    static Result<T> failure(const char* errorMessage);
    static Result<T> fromOptional(std::optional<T> value, const char* errorMessage);

    static Result<T> failuref(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
    static Result<T> incompletef(const char* fmt, ...) __attribute__((format(printf, 1, 2)));


    [[nodiscard]] bool isSuccess() const { return type == Type::Success; }
    [[nodiscard]] bool isError() const { return type == Type::Failure; }
    [[nodiscard]] bool isIncomplete() const { return type == Type::Incomplete; }

    [[nodiscard]] const T& getValueConst() const { return value.value(); }
    [[nodiscard]] T& getValue() { return value.value(); }
    [[nodiscard]] const char* getError() const { return errorMessage_; }
    [[nodiscard]] Result::Type getStatus() const;

private:
    std::optional<T> value;
    constexpr static size_t ERROR_MESSAGE_SIZE = 512;
    char errorMessage_[ERROR_MESSAGE_SIZE]{}; // fixed-size buffer
    Type type;


    Result(std::optional<T> value, const char* errorMessage, Type type);
};

template <>
class Result<void>
{
public:
    enum class Type { Success = 0, Incomplete = 1, Failure = 2 };

    static Result<void> success();
    static Result<void> pending(const char* errorMessage);
    static Result<void> failure(const char* errorMessage);

    // printf-style
    static Result<void> failuref(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
    static Result<void> incompletef(const char* fmt, ...) __attribute__((format(printf, 1, 2)));


    [[nodiscard]] bool isSuccess() const { return type == Type::Success; }
    [[nodiscard]] bool isError() const { return type == Type::Failure; }
    [[nodiscard]] bool isIncomplete() const { return type == Type::Incomplete; }
    [[nodiscard]] const char* getError() const { return errorMessage_; }
    [[nodiscard]] Result::Type getStatus() const;

private:
    constexpr static size_t ERROR_MESSAGE_SIZE = 512;
    char errorMessage_[ERROR_MESSAGE_SIZE]{}; // fixed-size buffer

    Type type;

    Result(const char* errorMessage, Type type);
};

// ----- Macros para Result<T> con ClassName -----
#define RESULT_CLASS_FAILUREF(ClassType, fmt, ...) \
Result<ClassType>::failuref("%s" fmt, getClassNameCString(), ##__VA_ARGS__)

#define RESULT_CLASS_INCOMPLETEF(ClassType, fmt, ...) \
Result<ClassType>::incompletef("%s" fmt, getClassNameCString(), ##__VA_ARGS__)


// ----- Macros para Result<void> con ClassName -----
#define RESULT_CLASS_VOID_FAILUREF(fmt, ...) \
Result<void>::failuref("%s" fmt, getClassNameCString(), ##__VA_ARGS__)

#define RESULT_CLASS_VOID_INCOMPLETEF(fmt, ...) \
Result<void>::incompletef("%s" fmt, getClassNameCString(), ##__VA_ARGS__)

// ----- Macros generales para Result<T> sin ClassName -----
#define RESULT_FAILUREF(ClassType, fmt, ...) \
    Result<ClassType>::failuref(fmt, ##__VA_ARGS__)

#define RESULT_INCOMPLETEF(ClassType, fmt, ...) \
    Result<ClassType>::incompletef(fmt, ##__VA_ARGS__)

// ----- Macros generales para Result<void> sin ClassName -----
#define RESULT_VOID_FAILUREF(fmt, ...) \
    Result<void>::failuref(fmt, ##__VA_ARGS__)

#define RESULT_VOID_INCOMPLETEF(fmt, ...) \
    Result<void>::incompletef(fmt, ##__VA_ARGS__)


// ----- Macros SUCCESS para Result<T> y Result<void>, con o sin ClassName -----
#define RESULT_SUCCESS(ClassType, value) \
Result<ClassType>::success(value)

#define RESULT_VOID_SUCCESS() \
Result<void>::success()


#endif // RESULT_H
