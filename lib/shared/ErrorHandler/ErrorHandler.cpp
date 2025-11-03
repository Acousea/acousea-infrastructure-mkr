#include "ErrorHandler.h"

#include "Logger/Logger.h"
#include <cstdarg>

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cstdio>
#include <thread>
#include <chrono>
#include <cstdlib>  // std::abort
#endif

// ------------------------------------------------------------------
// Salida mínima sin depender de Logger (en caso de fallo total)
// ------------------------------------------------------------------
static void rawPrintLine(const char* msg)
{
#ifdef ARDUINO
    SerialUSB.println(msg);
    SerialUSB.flush();
#else
    std::fprintf(stderr, "%s\n", msg);
    std::fflush(stderr);
#endif
}

void ErrorHandler::setHandler(const ErrorHandlerCallback handler)
{
    customHandler = handler;
}


void ErrorHandler::handleError(const char* msg)
{
    LOG_CLASS_ERROR("HANDLING ERROR: %s", msg);

    if (customHandler)
        customHandler(msg);
    else
        defaultHandler(msg);

    // exit(1);
}

// ------------------------------------------------------------------
// Handle error (printf-style formatted)
// ------------------------------------------------------------------
void ErrorHandler::handleErrorf(const char* fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    handleError(buffer);
}


void ErrorHandler::defaultHandler(const char* msg)
{
    LOG_CLASS_ERROR("-> Default handler triggered.");
    performHardwareReset();
}

#ifdef ARDUINO

void ErrorHandler::performHardwareReset()
{
    LOG_CLASS_ERROR("-> Performing hardware reset (Arduino).");
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW); // Bring the RST pin LOW
    delay(100); // Ensure sufficient time for reset
    // El microcontrolador se reinicia aquí; no hace falta más.
}

#else // Implementación nativa (Linux/Windows/macOS)
void ErrorHandler::performHardwareReset()
{
    LOG_CLASS_ERROR("-> Performing hardware reset (native).");
    rawPrintLine("ERROR_HANDLER -> Simulating hardware reset (native). Terminating process.");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::abort();
}
#endif
