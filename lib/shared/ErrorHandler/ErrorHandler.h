#ifndef ACOUSEA_ERRORHANDLER_H
#define ACOUSEA_ERRORHANDLER_H

#define RESET_PIN 7


#include <string>
#include <functional>
#include "Logger/Logger.h"

// Define a callback type for handling errors
using ErrorHandlerCallback = std::function<void(const std::string &)>;

class ErrorHandler {
public:
    // Set a custom error handling callback
    static void setHandler(ErrorHandlerCallback handler);

    // Handle an error using the custom callback or default implementation
    static void handleError(const std::string &errorMessage);

private:
    static inline ErrorHandlerCallback customHandler = nullptr;

    // Default error handling
    static void defaultHandler(const std::string &errorMessage);

    static void performHardwareReset();
};

#endif // ACOUSEA_ERRORHANDLER_H
