#ifndef ACOUSEA_ERRORHANDLER_H
#define ACOUSEA_ERRORHANDLER_H


#include "ClassName.h"

// Define a callback type for handling errors
using ErrorHandlerCallback = void (*)(const char*);

class ErrorHandler
{
    CLASS_NAME(ErrorHandler)

public:
    // Set a custom error handling callback
    static void setHandler(ErrorHandlerCallback handler);

    // Maneja un error con formato printf-style
    [[noreturn]] static void handleErrorf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

    // Maneja un error simple sin formato
    [[noreturn]] static void handleError(const char* msg);

private:
    static inline ErrorHandlerCallback customHandler = nullptr;

    // Default error handling
    static void defaultHandler(const char* msg);
    static void performHardwareReset();
};

#define ERROR_HANDLE_CLASS(fmt, ...) \
    ErrorHandler::handleErrorf("%s" fmt, getClassNameCString(), ##__VA_ARGS__)

#define ERROR_HANDLE(fmt, ...) \
    ErrorHandler::handleErrorf(fmt, ##__VA_ARGS__)


#endif // ACOUSEA_ERRORHANDLER_H
