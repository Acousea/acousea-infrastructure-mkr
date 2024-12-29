#include "Logger.h"

void Logger::initialize(SDManager *sdManager, const char *logFilePath, Mode mode) {
    Logger::sdManager = sdManager;
    Logger::logFilePath = logFilePath;
    Logger::mode = mode;
}


void Logger::logError(const std::string &errorMessage) {
    log("ERROR", errorMessage);
}

void Logger::logInfo(const std::string &infoMessage) {
    log("INFO", infoMessage);
}

void Logger::log(const std::string &logType, const std::string &message) {
    switch (mode) {
        case Mode::SDCard:
            if (sdManager) {
                logToSDCard(logType, message);
            } else {
                logToSerial(logType, message);
            }
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

std::string Logger::vectorToHexString(const std::vector<unsigned char> &data) {
    std::string result = "";
    for (const auto &byte: data) {
        char buffer[3];
        sprintf(buffer, "%02X", byte);
        result += buffer;
    }
    return result;
}

void Logger::setCurrentTime(time_t time) {
    Logger::currentTime = time;
}

std::string Logger::getTimestampString() {
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
    return std::string(buffer);
}

void Logger::logToSerial(const std::string &logType, const std::string &message) {
    Serial.println(("[" + getTimestampString() + "] " + logType + ": " + message).c_str());
}

void Logger::logToSDCard(const std::string &logType, const std::string &message) {
    if (!sdManager) return;

    std::string entry = getTimestampString() + "," + logType + "," + message + "\n";
    if (!sdManager->appendToFile(logFilePath, entry.c_str())) {
        Serial.println("Logger::logToSDCard() -> Failed to append to log file.");
    }
}
