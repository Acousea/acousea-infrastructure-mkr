#ifndef ACOUSEA_LOGGER_H
#define ACOUSEA_LOGGER_H


#include <ctime>
#include <cstdarg>  // para va_list, va_start, va_end

#include "StorageManager/StorageManager.hpp"
#include "RTCController.hpp"
#include "IDisplay.h"


class Logger
{
public:
    // Configuración del Logger: modo de operación
    enum class Mode
    {
        SDCard,
        SerialOnly,
        Both
    };

#ifdef LOGGER_HEXSTRING_DYNAMIC
    struct HexString
    {
        char* buffer = nullptr;
        size_t size = 0;
        ~HexString() { if (buffer) free(buffer); }
        [[nodiscard]] const char* c_str() const { return buffer ? buffer : ""; }
    };
#else
    struct HexString
    {
        static constexpr size_t MAX_SIZE = 1024;
        char buffer[MAX_SIZE]{};
        [[nodiscard]] const char* c_str() const { return buffer; }
    };

#endif


    static void initialize(
        IDisplay* display_,
        StorageManager* sdManager_,
        RTCController* rtc_,
        const char* logFilePath_,
        Mode mode_ = Mode::SerialOnly);

#ifdef PLATFORM_ARDUINO
    static void logfFreeMemory(const char* fmt = "", ...) __attribute__((format(printf, 1, 2)));
#endif
    static void logInfo(const char* message);
    static void logWarning(const char* message);
    static void logError(const char* message);

    static void logfInfo(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
    static void logfWarning(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
    static void logfError(const char* fmt, ...) __attribute__((format(printf, 1, 2)));


    static void vectorToHexString(const unsigned char* data, size_t length, char* outBuffer, size_t outSize);
    static HexString vectorToHexString(const unsigned char* data, size_t length);

private:
    static inline IDisplay* display = nullptr;
    static inline StorageManager* storageManager = nullptr;
    static inline const char* logFilePath = nullptr;
    static inline Mode mode = Mode::SerialOnly;
    static inline RTCController* rtc = nullptr;
    static inline time_t currentTime = 0; // tiempo actual en epoch


    // Buffer compartido global para todos los métodos (reduce uso de stack)
    static constexpr size_t SHARED_BUFFER_SIZE =
#ifdef PLATFORM_ARDUINO
        1024;
#else
    2048;
#endif
    static inline char sharedBuffer[SHARED_BUFFER_SIZE]{};

    static void getTimestamp(char* buffer, size_t len);

    static void vlog(const char* logType, const char* fmt, va_list args);
    static void vlog(const char* logType, const char* message);

    static bool clearLog();

    static void do_log();

    static void logToSerial(const char* line);
    static void logToSDCard(const char* line);
};

// ============================================================================
// Logging convenience macros (safe fallback when CLASS_NAME is missing)
// ============================================================================

#ifndef ACOUSEA_LOGGER_MACROS
#define ACOUSEA_LOGGER_MACROS

// ---------- Class-aware logging (uses CLASS_NAME) ----------
#define LOG_CLASS_INFO(fmt, ...) \
Logger::logfInfo("%s" fmt, getClassNameCString(), ##__VA_ARGS__)

#define LOG_CLASS_WARNING(fmt, ...) \
Logger::logfWarning("%s" fmt, getClassNameCString(), ##__VA_ARGS__)

#define LOG_CLASS_ERROR(fmt, ...) \
Logger::logfError("%s" fmt, getClassNameCString(), ##__VA_ARGS__)

// ---------- Global / free-function logging (no class prefix) ----------
#define LOG_INFO(fmt, ...) \
Logger::logfInfo(fmt, ##__VA_ARGS__)

#define LOG_WARNING(fmt, ...) \
Logger::logfWarning(fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
Logger::logfError(fmt, ##__VA_ARGS__)

// ---------- Free memory logging (Arduino-only) ----------
#ifdef PLATFORM_ARDUINO
#define LOG_FREE_MEMORY(fmt, ...) \
Logger::logfFreeMemory(fmt, ##__VA_ARGS__)
#else
#define LOG_FREE_MEMORY(fmt, ...) \
((void)0)
#endif


#endif // ACOUSEA_LOGGER_MACROS

#endif // ACOUSEA_LOGGER_H
