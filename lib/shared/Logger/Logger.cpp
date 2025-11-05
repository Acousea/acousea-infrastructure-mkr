#include "Logger.h"

#include <malloc.h> // mallinfo
#include <cstdarg>  // para va_list, va_start, va_end
#include <cstring>

#ifdef ARDUINO
#include <Arduino.h>

extern "C" char* sbrk(int incr);

int freeMemoryStackVsHeap()
{
    char top;
    return &top - reinterpret_cast<char*>(sbrk(0));
}

void Logger::logfFreeMemory(const char* fmt, ...)
{
    struct mallinfo mi = mallinfo();
    const int freeMem = freeMemoryStackVsHeap();

    char prefix[128];
    if (fmt && *fmt)
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(prefix, sizeof(prefix), fmt, args);
        va_end(args);
    }
    else
    {
        prefix[0] = '\0';
    }

    logfInfo("%s Free stack/heap gap: %d bytes, Heap total: %d bytes, Used: %d, Free reusable: %d",
             prefix, freeMem, mi.arena, mi.uordblks, mi.fordblks);
}
#endif

void Logger::initialize(IDisplay* display_, StorageManager* sdManager_, RTCController* rtc_, const char* logFilePath_,
                        Mode mode_)
{
    Logger::display = display_;
    Logger::storageManager = sdManager_;
    Logger::rtc = rtc_;
    Logger::logFilePath = logFilePath_;
    Logger::mode = mode_;

    const bool validPath = (logFilePath_ && strlen(logFilePath_) <= 8);

    char msg[64];
    snprintf(msg, sizeof(msg),
             validPath ? "[Logger] Log file path: %s" : "[Logger] Invalid path (must follow 8.3 format).",
             validPath ? logFilePath_ : "");

#if defined(ARDUINO)
    if (display)
        display->print(msg);
    else
        Serial.println(msg);
#else
    if (display)
        display->print(msg);
    else
        printf("%s\n", msg);
#endif
}

void Logger::logError(const char* message)
{
    if (display)
        display->setColor(IDisplay::Color::RED);
    log("ERROR", message);
}

void Logger::logWarning(const char* message)
{
    if (display)
        display->setColor(IDisplay::Color::ORANGE);
    log("WARNING", message);
}

void Logger::logInfo(const char* message)
{
    if (display)
        display->setColor(IDisplay::Color::DEFAULT);
    log("INFO", message);
}

void Logger::logfError(const char* fmt, ...)
{
    if (display)
        display->setColor(IDisplay::Color::RED);

    va_list args;
    va_start(args, fmt);
    vsnprintf(sharedBuffer, sizeof(sharedBuffer), fmt, args);
    va_end(args);

    log("ERROR", sharedBuffer);
}

void Logger::logfWarning(const char* fmt, ...)
{
    if (display)
        display->setColor(IDisplay::Color::ORANGE);

    va_list args;
    va_start(args, fmt);
    vsnprintf(sharedBuffer, sizeof(sharedBuffer), fmt, args);
    va_end(args);

    log("WARNING", sharedBuffer);
}

void Logger::logfInfo(const char* fmt, ...)
{
    if (display)
        display->setColor(IDisplay::Color::DEFAULT);

    va_list args;
    va_start(args, fmt);
    vsnprintf(sharedBuffer, sizeof(sharedBuffer), fmt, args);
    va_end(args);

    log("INFO", sharedBuffer);
}


void Logger::log(const char* logType, const char* message)
{
    switch (mode)
    {
    case Mode::SDCard:
        logToSDCard(logType, message);
        break;
    case Mode::SerialOnly:
        logToSerial(logType, message);
        break;
    case Mode::Both:
        logToSerial(logType, message);
        logToSDCard(logType, message);
        break;
    }
}

bool Logger::clearLog()
{
    if (mode == Mode::SDCard && storageManager)
    {
        return storageManager->overwriteFile(logFilePath, "");
    }
    return false;
}

void Logger::vectorToHexString(const unsigned char* data, const size_t length, char* outBuffer, const size_t outSize)
{
    size_t needed = (length * 2) + 1;
    if (outSize < needed)
        return;
    for (size_t i = 0; i < length; ++i)
        sprintf(outBuffer + i * 2, "%02X", data[i]);
    outBuffer[length * 2] = '\0';
}

Logger::HexString Logger::vectorToHexString(const unsigned char* data, const size_t length)
{
#ifdef LOGGER_HEXSTRING_DYNAMIC
    HexString hex{};
    hex.size = (length * 2) + 1;
    hex.buffer = static_cast<char*>(malloc(hex.size));
    if (!hex.buffer)
        return hex;
    for (size_t i = 0; i < length; ++i)
        sprintf(hex.buffer + i * 2, "%02X", data[i]);
    hex.buffer[length * 2] = '\0';
    return hex;
#else
    HexString hex{};
    constexpr size_t max = HexString::MAX_SIZE;
    for (size_t i = 0; i < length && (i * 2 + 2) < max; ++i)
        sprintf(hex.buffer + i * 2, "%02X", data[i]);
    hex.buffer[(length * 2) < max ? (length * 2) : (max - 1)] = '\0';
    return hex;
#endif
}

void Logger::getTimestamp(char* buffer, const size_t len)
{
    time_t epoch_t = rtc ? static_cast<time_t>(rtc->getEpoch()) : time(nullptr);

    const tm* t = localtime(&epoch_t);
    if (!t)
    {
        // Fallback en caso raro de fallo de localtime()
        snprintf(buffer, len, "[INVALID TIME]");
        return;
    }
    snprintf(buffer, len, "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
}

void Logger::logToSerial(const char* logType, const char* message)
{
    char ts[20];
    getTimestamp(ts, sizeof(ts));

    snprintf(sharedBuffer, sizeof(sharedBuffer) - 1, "[%s] %s: %s", ts, logType, message);
    sharedBuffer[sizeof(sharedBuffer) - 1] = '\0';

    if (display)
        display->print(sharedBuffer);
    else
#if defined(ARDUINO)
        Serial.println(sharedBuffer);
#else
        printf("%s\n", sharedBuffer);
#endif
}


void Logger::logToSDCard(const char* logType, const char* message)
{
    if (!storageManager)
    {
        if (display)
            display->print("Logger::logToSDCard() -> StorageManager not initialized.");
        return;
    }

    char ts[20];
    getTimestamp(ts, sizeof(ts));

    char entry[1024];
    snprintf(entry, sizeof(entry), "%s,%s,%s\n", ts, logType, message);

    if (!storageManager->appendToFile(logFilePath, entry))
    {
        if (display)
            display->print("Logger::logToSDCard() -> Failed to append to log file.");
    }
}
