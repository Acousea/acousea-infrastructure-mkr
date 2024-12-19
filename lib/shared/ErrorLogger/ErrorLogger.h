#ifndef ACOUSEA_MKR1310_NODES_ERRORLOGGER_H
#define ACOUSEA_MKR1310_NODES_ERRORLOGGER_H

#include <string>
#include <ctime>
#include "ArduinoJson.h"
#include "SDManager/SDManager.h"

class ErrorLogger {
public:
    explicit ErrorLogger(SDManager &sdManager, const char *logFilePath);

    void logError(const std::string &errorMessage);

    void printErrorLog() const;

    bool clearLog();

private:
    SDManager &sdManager;
    const char *logFilePath;

    [[nodiscard]] std::string getTimestamp() const;
};


#endif //ACOUSEA_MKR1310_NODES_ERRORLOGGER_H
