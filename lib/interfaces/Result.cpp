// Result.cpp
#include "Result.h"

#include <vector>
#include <cstdarg>   // para va_list, va_start, va_end
#include <cstdio>

#include "bindings/nodeDevice.pb.h"

template <typename T>
Result<T>::Result(std::optional<T> value, const char* errorMessage, const Type type)
    : value(std::move(value)), type(type)
{
    if (errorMessage)
    {
        strncpy(errorMessage_, errorMessage, sizeof(errorMessage_) - 1);
        errorMessage_[sizeof(errorMessage_) - 1] = '\0';
    }
    else
    {
        errorMessage_[0] = '\0';
    }
}


template <typename T>
Result<T> Result<T>::success(T value)
{
    return Result<T>(value, "", Type::Success);
}

template <typename T>
Result<T> Result<T>::pending(const char* errorMessage)
{
    return Result<T>(std::nullopt, errorMessage, Type::Incomplete);
}

template <typename T>
Result<T> Result<T>::failure(const char* errorMessage)
{
    return Result<T>(std::nullopt, errorMessage, Type::Failure);
}


template <typename T>
Result<T> Result<T>::fromOptional(std::optional<T> value, const char* errorMessage)
{
    if (value.has_value()) return success(value.value());
    return failure(errorMessage);
}

template <typename T>
Result<T> Result<T>::failuref(const char* fmt, ...)
{
    char msg[ERROR_MESSAGE_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    return failure(msg);
}

template <typename T>
Result<T> Result<T>::incompletef(const char* fmt, ...)
{
    char msg[ERROR_MESSAGE_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    return pending(msg);
}

template <typename T>
typename Result<T>::Type Result<T>::getStatus() const
{
    return type;
}


// ----- Result<void> specialization -----


// ----- Result<void> specialization -----
Result<void>::Result(const char* errorMessage, Type type)
    : type(type)
{
    if (errorMessage)
    {
        strncpy(errorMessage_, errorMessage, sizeof(errorMessage_) - 1);
        errorMessage_[sizeof(errorMessage_) - 1] = '\0';
    }
    else
    {
        errorMessage_[0] = '\0';
    }
}


Result<void> Result<void>::success()
{
    return Result<void>("", Type::Success);
}

Result<void> Result<void>::pending(const char* errorMessage)
{
    return Result<void>(errorMessage, Type::Incomplete);
}

Result<void> Result<void>::failure(const char* errorMessage)
{
    return Result<void>(errorMessage, Type::Failure);
}


// 🔥 printf-style versions
Result<void> Result<void>::failuref(const char* fmt, ...)
{
    char msg[ERROR_MESSAGE_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    return failure(msg);
}

Result<void> Result<void>::incompletef(const char* fmt, ...)
{
    char msg[ERROR_MESSAGE_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    return pending(msg);
}

Result<void>::Type Result<void>::getStatus() const
{
    return type;
}


// ----- Explicit instantiations -----
template class Result<int>;
template class Result<float>;
template class Result<double>;
template class Result<size_t>;
template class Result<acousea_CommunicationPacket>;
template class Result<acousea_CommunicationPacket*>;
template class Result<acousea_NodeConfiguration>;
template class Result<acousea_OperationMode>;
template class Result<acousea_ReportingPeriodEntry>;
template class Result<acousea_ReportType>;
template class Result<acousea_ModuleWrapper>;

template class Result<std::vector<uint8_t>>;
// template class Result<std::vector<unsigned char>>;
