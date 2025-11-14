#include "Logger.h"

#include <malloc.h> // mallinfo

#include <cstring>
#include <cstdio>

#ifdef PLATFORM_ARDUINO
#include <Arduino.h>

extern "C" char* sbrk(int incr);

int heapEnd()
{
    // The sbrk(0) function returns the current end of the heap (grows upwards)
    return reinterpret_cast<int>(sbrk(0));
}

int stackTop()
{
    // The address of a local variable gives an approximation of the current top of the stack (grows downwards)
    char top;
    return reinterpret_cast<int>(&top);
}

void Logger::logfFreeMemory(const char* fmt, ...)
{
    // --- Recolectar información de memoria ---
    const struct mallinfo mi = mallinfo();
    const int topOfStack = stackTop();
    const int endOfHeap = heapEnd();
    const int freeMem = topOfStack - endOfHeap;

    // Format everything into sharedBuffer, as in vlog()
    char timestamp[20];
    getTimestamp(timestamp, sizeof(timestamp));

    // Formateamos el encabezado de DEBUG
    const int headerLen = snprintf(sharedBuffer, sizeof(sharedBuffer), "[%s] DEBUG ", timestamp);
    if (headerLen < 0 || static_cast<size_t>(headerLen) >= sizeof(sharedBuffer))
    {
        // Error al formatear el encabezado o se ha excedido el tamaño del buffer
        if (display) display->setColor(IDisplay::Color::RED);
        strcpy(sharedBuffer, "[LOGGER INTERNAL ERROR] vlog() header formatting failed.");
        do_log();
        return;
    }

    // Formateamos el resto de la información de memoria
    va_list args;
    va_start(args, fmt);

    // Formatear el mensaje con los argumentos variables
    int messageLen = vsnprintf(sharedBuffer + headerLen, sizeof(sharedBuffer) - static_cast<size_t>(headerLen), fmt, args);
    if (messageLen < 0 || messageLen >= (sizeof(sharedBuffer) - headerLen))
    {
        // Error al formatear el mensaje con los parámetros
        if (display) display->setColor(IDisplay::Color::RED);
        strcpy(sharedBuffer, "[LOGGER INTERNAL ERROR] vlog() message formatting failed.");
        do_log();
        return;
    }

    // Ahora agregamos la información sobre la memoria (stack, heap, etc.)
    snprintf(sharedBuffer + headerLen + messageLen, sizeof(sharedBuffer) - static_cast<size_t>(headerLen + messageLen),
        " Free memory: Free stack/heap gap: %d bytes [STACK_TOP = 0x%X, HEAP_END = 0x%X], "
        "Heap total: %d bytes, Used: %d, Free reusable: %d",
        freeMem,       // Memoria libre entre heap y stack
        topOfStack,    // Dirección superior de la pila
        endOfHeap,     // Dirección final del heap
        mi.arena,      // Total de memoria de heap utilizada
        mi.uordblks,   // Memoria usada
        mi.fordblks    // Memoria libre
    );

    va_end(args);

    if (display) display->setColor(IDisplay::Color::ORANGE);

    // Salida según modo
    do_log();
}


#endif

void Logger::initialize(IDisplay* display_,
                        StorageManager* sdManager_,
                        RTCController* rtc_,
                        const char* logFilePath_,
                        const Mode mode_)
{
    // Limpieza del buffer global (evita residuos entre logs o tests)
    memset(sharedBuffer, 0, sizeof(sharedBuffer));

    Logger::display = display_;
    Logger::storageManager = sdManager_;
    Logger::rtc = rtc_;
    Logger::logFilePath = logFilePath_;
    Logger::mode = mode_;

    const bool validPath = (logFilePath_ && strlen(logFilePath_) <= 8);

    // Usamos el buffer compartido global para evitar stack local
    snprintf(sharedBuffer, sizeof(sharedBuffer),
             validPath
                 ? "[Logger] Log file path: %s"
                 : "[Logger] Invalid path (must follow 8.3 format).",
             validPath ? logFilePath_ : "");

#if defined(ARDUINO)
    if (display)
        display->print(sharedBuffer);
    else
        Serial.println(sharedBuffer);
#else
    if (display)
        display->print(sharedBuffer);
    else
        printf("%s\n", sharedBuffer);
#endif
}

void Logger::logInfo(const char* message)
{
    if (display)
        display->setColor(IDisplay::Color::DEFAULT);

    vlog("INFO", message);
}

void Logger::logWarning(const char* message)
{
    if (display)
        display->setColor(IDisplay::Color::ORANGE);

    vlog("WARNING", message);
}

void Logger::logError(const char* message)
{
    if (display)
        display->setColor(IDisplay::Color::RED);

    vlog("ERROR", message);
}


void Logger::logfInfo(const char* fmt, ...)
{
    if (display)
        display->setColor(IDisplay::Color::DEFAULT);

    va_list args;
    va_start(args, fmt);
    vlog("INFO", fmt, args);
    va_end(args);
}

void Logger::logfWarning(const char* fmt, ...)
{
    if (display)
        display->setColor(IDisplay::Color::ORANGE);

    va_list args;
    va_start(args, fmt);
    vlog("WARNING", fmt, args);
    va_end(args);
}

void Logger::logfError(const char* fmt, ...)
{
    if (display)
        display->setColor(IDisplay::Color::RED);

    va_list args;
    va_start(args, fmt);
    vlog("ERROR", fmt, args);
    va_end(args);
}


void Logger::do_log()
{
    switch (mode)
    {
    case Mode::SerialOnly:
        logToSerial(sharedBuffer);
        break;
    case Mode::SDCard:
        logToSDCard(sharedBuffer);
        break;
    case Mode::Both:
        logToSerial(sharedBuffer);
        logToSDCard(sharedBuffer);
        break;
    }
}

void Logger::vlog(const char* loggingClass, const char* message)
{
    char timestamp[20];
    getTimestamp(timestamp, sizeof(timestamp));

    snprintf(sharedBuffer, sizeof(sharedBuffer), "[%s] %s: %s", timestamp, loggingClass, message);

    do_log();
}


void Logger::vlog(const char* loggingClass, const char* fmt, va_list& args)
{
    char timestamp[20];
    getTimestamp(timestamp, sizeof(timestamp));

    const int headerLen = snprintf(sharedBuffer, sizeof(sharedBuffer), "[%s] %s: ", timestamp, loggingClass);
    if (headerLen < 0 || static_cast<size_t>(headerLen) >= sizeof(sharedBuffer))
    {
        // Error al formatear el encabezado o se ha excedido el tamaño del buffer
        display->setColor(IDisplay::Color::RED);
        strcpy(sharedBuffer, "[LOGGER INTERNAL ERROR] vlog() header formatting failed.");
        do_log();
        return;
    }

    // Formatear el mensaje con los argumentos variables
    vsnprintf(sharedBuffer + headerLen, sizeof(sharedBuffer) - static_cast<size_t>(headerLen), fmt, args);

    // Salida según modo
    do_log();
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

// FIXME: Too much memory usage with large buffers?
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
    constexpr size_t max = HexString::MAX_LOGGER_BUF_SIZE;
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


bool Logger::clearLog()
{
    if (mode == Mode::SDCard && storageManager)
    {
        return storageManager->deleteFile(logFilePath) && storageManager->createEmptyFile(logFilePath);
    }
    return false;
}


void Logger::logToSerial(const char* line)
{
#if defined(ARDUINO)
    if (display)
        display->print(line);
    else
        Serial.println(line);
#else
    if (display)
        display->print(line);
    else
        printf("%s\n", line);
#endif
}


void Logger::logToSDCard(const char* line)
{
    if (!storageManager || !logFilePath)
    {
        if (display)
            display->print("Logger::logToSDCard() -> StorageManager not initialized.");
        return;
    }

    const size_t len = strlen(line);
    if (len == 0)
        return;

    storageManager->appendBytesToFile(logFilePath, reinterpret_cast<const uint8_t*>(line), len);

    // Añadimos salto de línea final si no lo tiene
    if (line[len - 1] != '\n')
    {
        storageManager->appendBytesToFile(logFilePath, reinterpret_cast<const uint8_t*>("\n"), 1);
    }
}
