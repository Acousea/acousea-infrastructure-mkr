#ifndef ACOUSEA_MKR1310_NODES_LOGGER_H
#define ACOUSEA_MKR1310_NODES_LOGGER_H

#include <string>
#include <ctime>
#include "SDManager/SDManager.h"
#include <Arduino.h>
#include <vector>

class Logger {
public:
    // Configuración del Logger: modo de operación
    enum class Mode {
        SDCard,
        SerialOnly,
        Both
    };

    static void initialize(SDManager *sdManager, const char *logFilePath, Mode mode = Mode::SerialOnly);

    static void logError(const std::string &errorMessage);

    static void logInfo(const std::string &infoMessage);

    static void printLog();

    static bool clearLog();

    static std::string vectorToHexString(const std::vector<unsigned char> &data);


private:
    static inline SDManager *sdManager = nullptr;
    static inline const char *logFilePath = nullptr;
    static inline Mode mode = Mode::SerialOnly;
    static inline std::time_t currentTime = 0;

    static std::string getTimestampString();

public:
    static void setCurrentTime(time_t time);

private:

    static void logToSerial(const std::string &logType, const std::string &message);

    static void logToSDCard(const std::string &logType, const std::string &message);

    static void log(const std::string &logType, const std::string &message);
};

#endif // ACOUSEA_MKR1310_NODES_LOGGER_H