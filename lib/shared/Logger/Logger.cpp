#include "Logger.h"

void Logger::initialize(SDManager *sdManager, const char *logFilePath, Mode mode) {
    Logger::sdManager = sdManager;
    Logger::logFilePath = logFilePath;
    Logger::mode = mode;
}

void Logger::logError(const std::string &errorMessage) {
    std::string logType = "ERROR";
    if (mode == Mode::SDCard && sdManager) {
        logToSDCard(logType, errorMessage);
    } else {
        logToSerial(logType, errorMessage);
    }
}

void Logger::logInfo(const std::string &infoMessage) {
    std::string logType = "INFO";
    if (mode == Mode::SDCard && sdManager) {
        logToSDCard(logType, infoMessage);
    } else {
        logToSerial(logType, infoMessage);
    }
}

void Logger::printLog() {
    if (mode == Mode::SDCard && sdManager) {
        String content = sdManager->readFile(logFilePath);
        if (content.isEmpty()) {
            Serial.println("Logger::printLog() -> No logs available.");
            return;
        }
        Serial.println("Logger::printLog() -> Log Content:");
        Serial.println(content.c_str());
    } else {
        Serial.println("Logger::printLog() -> SD card logging not enabled.");
    }
}

bool Logger::clearLog() {
    if (mode == Mode::SDCard && sdManager) {
        return sdManager->overwriteFile(logFilePath, "");
    }
    return false;
}

std::string Logger::getTimestamp() {
    time_t now = time(nullptr);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buffer);
}

void Logger::logToSerial(const std::string &logType, const std::string &message) {
    Serial.println(("[" + getTimestamp() + "] " + logType + ": " + message).c_str());
}

void Logger::logToSDCard(const std::string &logType, const std::string &message) {
    if (!sdManager) return;

    std::string entry = getTimestamp() + "," + logType + "," + message + "\n";
    if (!sdManager->appendToFile(logFilePath, entry.c_str())) {
        Serial.println("Logger::logToSDCard() -> Failed to append to log file.");
    }
}
