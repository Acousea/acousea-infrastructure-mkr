#include "ErrorHandler.h"

#include "Logger/Logger.h"
#include <cstdarg>
#include <cstring>

#include "WatchDog/WatchDogUtils.hpp"

#ifdef PLATFORM_ARDUINO
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
#ifdef PLATFORM_ARDUINO
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
    Logger::vlog("ERROR_HANDLER", msg);

    if (customHandler) customHandler();
    performReset();
}

// ------------------------------------------------------------------
// Handle error (printf-style formatted)
// ------------------------------------------------------------------
void ErrorHandler::handleErrorf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Logger::vlog("ERROR_HANDLER", fmt, args);
    va_end(args);

    if (customHandler) customHandler();
    performReset();

}


#define RESET_MODE_WATCHDOG
#if defined(PLATFORM_ARDUINO) && defined(RESET_MODE_HARDWARE)

void ErrorHandler::performReset()
{
    rawPrintLine("-> Forcing HARDWARE reset (Arduino).");
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW); // Bring the RST pin LOW
}

#elif defined(PLATFORM_ARDUINO) && defined(RESET_MODE_WATCHDOG)
void ErrorHandler::performReset()
{
    WatchdogUtils::disable();
    WatchdogUtils::enable(2000); // Set a short timeout to trigger the reset quickly
    rawPrintLine("-> Forcing WATCHDOG reset (Arduino).");
    // WARNING: DO NOT USE waitFor(), since it internally resets the watchdog!
    delay(WatchdogUtils::getTimeout() + 1000); // Wait longer than the watchdog timeout
}

#else // Implementación nativa (Linux/Windows/macOS)
void ErrorHandler::performReset()
{
    LOG_CLASS_ERROR("-> Forcing abort (native).");
    rawPrintLine("ERROR_HANDLER -> Simulating hardware reset (native). Terminating process.");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::abort();
}
#endif
