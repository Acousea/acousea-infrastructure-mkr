#include "ErrorLogger.h"

ErrorLogger::ErrorLogger(SDManager &sdManager, const char *logFilePath)
        : sdManager(sdManager), logFilePath(logFilePath) {}

void ErrorLogger::logError(const std::string &errorMessage) {
    // Retrieve the current error log content
    String content = sdManager.readFile(logFilePath);
    JsonDocument doc;

    // Parse existing log or create a new one
    if (!content.isEmpty()) {
        DeserializationError error = deserializeJson(doc, content);
        if (error) {
            Serial.println("ErrorLogger::logError() -> Failed to parse existing log. Overwriting.");
            doc.clear();
        }
    }

    // Add the new error entry
    JsonObject newError = doc.add<JsonObject>();
    newError["timestamp"] = getTimestamp();
    newError["message"] = errorMessage;

    // Serialize and save the updated log
    String updatedContent;
    serializeJson(doc, updatedContent);

    if (!sdManager.overwriteFile(logFilePath, updatedContent.c_str())) {
        Serial.println("ErrorLogger::logError() -> Failed to save error log.");
    }
}


void ErrorLogger::printErrorLog() const {
    String content = sdManager.readFile(logFilePath);
    if (content.isEmpty()) {
        Serial.println("ErrorLogger::printErrorLog() -> No errors logged.");
        return;
    }

    Serial.println("ErrorLogger::printErrorLog() -> Error Log:");
    Serial.println(content.c_str());
}

bool ErrorLogger::clearLog() {
    return sdManager.overwriteFile(logFilePath, "");
}

std::string ErrorLogger::getTimestamp() const {
    time_t now = time(nullptr);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buffer);
}
