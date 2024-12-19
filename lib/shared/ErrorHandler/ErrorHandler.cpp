#include "ErrorHandler.h"

void ErrorHandler::setHandler(ErrorHandlerCallback handler) {
    customHandler = std::move(handler);
}

void ErrorHandler::setLogger(ErrorLogger *logger) {
    errorLogger = logger;
}

void ErrorHandler::handleError(const std::string &errorMessage) {
    if (errorLogger)
    {
        errorLogger->logError(errorMessage);
    }

    if (customHandler)
    {
        customHandler(errorMessage);
    }
    else
    {
        defaultHandler(errorMessage);
    }
}

void ErrorHandler::defaultHandler(const std::string &errorMessage) {
    Serial.print("Error: ");
    Serial.println(errorMessage.c_str());
    Serial.println("System is resetting...");
    return performHardwareReset();
}

void ErrorHandler::performHardwareReset() {
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW); // Lleva el pin RST a LOW
    delay(100);                   // Espera suficiente tiempo para asegurar el reinicio
    // El microcontrolador se reiniciará aquí, no hay necesidad de más instrucciones
}


