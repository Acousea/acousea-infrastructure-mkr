#ifndef ACOUSEA_MKR1310_NODES_ERRORHANDLER_H
#define ACOUSEA_MKR1310_NODES_ERRORHANDLER_H

#define RESET_PIN 5


#include <string>
#include <functional>
#include <utility>
#include "ErrorLogger/ErrorLogger.h"

// Define un tipo de callback para manejar errores
using ErrorHandlerCallback = std::function<void(const std::string &)>;

class ErrorHandler {
public:

    static void setLogger(ErrorLogger *logger);

    // Configura un callback personalizado
    static void setHandler(ErrorHandlerCallback handler);

    // Maneja un error usando el callback o la implementación predeterminada
    static void handleError(const std::string &errorMessage);

private:
    // Callback personalizado (opcional)
    static inline ErrorHandlerCallback customHandler = nullptr;
    static inline ErrorLogger *errorLogger = nullptr;

    // Manejo predeterminado de errores
    static void defaultHandler(const std::string &errorMessage);

    static void performHardwareReset();
};

#endif // ACOUSEA_MKR1310_NODES_ERRORHANDLER_H
