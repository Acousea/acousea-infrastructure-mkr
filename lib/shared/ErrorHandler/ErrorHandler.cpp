#include "ErrorHandler.h"


#include "Logger/Logger.h"

#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <cstdio>
  #include <thread>
  #include <chrono>
  #include <cstdlib>  // std::abort
#endif

// ------------------------------------------------------------------
//  Salida mínima, sin depender de Logger (aún no inicializado)
// ------------------------------------------------------------------
namespace {
    inline void rawPrintLine(const std::string& s) {
#ifdef ARDUINO
        // Imprime por Serial (en SAMD, suele mapear a USB CDC o UART según core)
        SerialUSB.println(s.c_str());
        SerialUSB.flush();
#else
        std::fprintf(stderr, "%s\n", s.c_str());
        std::fflush(stderr);
#endif
    }
} // namespace

void ErrorHandler::setHandler(ErrorHandlerCallback handler) {
    customHandler = std::move(handler);
}

void ErrorHandler::handleError(const std::string &errorMessage) {
    // Log the error using Logger
    // rawPrintLine("ERROR: " + errorMessage);
    Logger::logError("ERROR_HANDLER -> HANDLING ERROR: " + errorMessage + " - Performing hardware reset.");


    // Execute the custom handler if available
    if (customHandler) {
        customHandler(errorMessage);
    } else {
        defaultHandler(errorMessage);
    }
}

void ErrorHandler::defaultHandler(const std::string &errorMessage) {
    // Print the error to Serial (if Logger is in SerialOnly mode, this is redundant)
    // rawPrintLine("ERROR_HANDLER -> HANDLING ERROR: " + errorMessage + " - Performing hardware reset.");
    performHardwareReset();
}


#ifdef ARDUINO

void ErrorHandler::performHardwareReset() {
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW); // Bring the RST pin LOW
    delay(100);                   // Ensure sufficient time for reset
    // El microcontrolador se reinicia aquí; no hace falta más.
}

#else // Implementación nativa (Linux/Windows/macOS)
void ErrorHandler::performHardwareReset() {
    rawPrintLine("ERROR_HANDLER -> Simulating hardware reset (native). Terminating process.");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::abort();
}
#endif