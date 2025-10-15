// Result.cpp
#include "Result.h"

template <typename T>
Result<T>::Result(std::optional<T> value, std::string errorMessage, Type type)
    : value(std::move(value)), errorMessage(std::move(errorMessage)), type(type) {}


template <typename T>
Result<T> Result<T>::success(T value)
{
    return Result<T>(value, "", Type::Success);
}

template <typename T>
Result<T> Result<T>::pending(const std::string& errorMessage)
{
    return Result<T>(std::nullopt, errorMessage, Type::Incomplete);
}

template <typename T>
Result<T> Result<T>::failure(const std::string& errorMessage)
{
    return Result<T>(std::nullopt, errorMessage, Type::Failure);
}

template <typename T>
Result<T> Result<T>::fromOptional(std::optional<T> value, const std::string& errorMessage)
{
    if (value.has_value()) return success(value.value());
    return failure(errorMessage);
}

// ----- Result<void> specialization -----

Result<void>::Result(std::string errorMessage, Type type)
    : errorMessage(std::move(errorMessage)), type(type) {}

Result<void> Result<void>::success()
{
    return Result<void>("", Type::Success);
}

Result<void> Result<void>::pending(const std::string& errorMessage)
{
    return Result<void>(errorMessage, Type::Incomplete);
}

Result<void> Result<void>::failure(const std::string& errorMessage)
{
    return Result<void>(errorMessage, Type::Failure);
}

// ----- Explicit instantiations -----
template class Result<int>;
template class Result<float>;
template class Result<double>;
