#include "ErrorHandler.h"

void ErrorHandler::setHandler(ErrorHandlerCallback handler) {
    customHandler = std::move(handler);
}

void ErrorHandler::handleError(const std::string &errorMessage) {
    // Log the error using Logger
    Logger::logError(errorMessage);

    // Execute the custom handler if available
    if (customHandler) {
        customHandler(errorMessage);
    } else {
        defaultHandler(errorMessage);
    }
}

void ErrorHandler::defaultHandler(const std::string &errorMessage) {
    // Print the error to Serial (if Logger is in SerialOnly mode, this is redundant)
    Serial.print("Error: ");
    Serial.println(errorMessage.c_str());

    // Perform a hardware reset
    Serial.println("System is resetting...");
    performHardwareReset();
}

void ErrorHandler::performHardwareReset() {
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW); // Bring the RST pin LOW
    delay(100);                   // Ensure sufficient time for reset
    // The microcontroller will reset here; no further instructions are needed
}
