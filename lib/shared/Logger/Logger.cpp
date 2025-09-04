#include "Logger.h"

void Logger::initialize(
    IDisplay* display, StorageManager* sdManager, const char* logFilePath, Mode mode)
{
    Logger::display = display;
    Logger::storageManager = sdManager;
    Logger::logFilePath = logFilePath;
    Logger::mode = mode;
}


void Logger::logError(const std::string& errorMessage)
{
    display->setColor(IDisplay::Color::RED);
    log("ERROR", errorMessage);
}

void Logger::logWarning(const std::string& warningMessage)
{
    display->setColor(IDisplay::Color::ORANGE);
    log("WARNING", warningMessage);
}

void Logger::logInfo(const std::string& infoMessage)
{
    display->setColor(IDisplay::Color::DEFAULT);
    log("INFO", infoMessage);
}

void Logger::log(const std::string& logType, const std::string& message)
{
    switch (mode)
    {
    case Mode::SDCard:
        if (storageManager)
        {
            logToSDCard(logType, message);
        }
        else
        {
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


void Logger::printLog()
{
    if (mode == Mode::SDCard && storageManager)
    {
        std::string content = storageManager->readFile(logFilePath);
        if (content.empty())
        {
            display->print("Logger::printLog() -> No logs available.");
            return;
        }
        display->print("Logger::printLog() -> Log Content:");
        display->print(content.c_str());
    }
    else
    {
        display->print("Logger::printLog() -> SD card logging not enabled.");
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

std::string Logger::vectorToHexString(const std::vector<unsigned char>& data)
{
    std::string result = "";
    for (const auto& byte : data)
    {
        char buffer[3];
        sprintf(buffer, "%02X", byte);
        result += buffer;
    }
    return result;
}

void Logger::setCurrentTime(time_t time)
{
    Logger::currentTime = time;
}

std::string Logger::getTimestampString()
{
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
    return std::string(buffer);
}

void Logger::logToSerial(const std::string& logType, const std::string& message)
{
    display->print(("[" + getTimestampString() + "] " + logType + ": " + message).c_str());
}

void Logger::logToSDCard(const std::string& logType, const std::string& message)
{
    if (!storageManager) return;

    std::string entry = getTimestampString() + "," + logType + "," + message + "\n";
    if (!storageManager->appendToFile(logFilePath, entry.c_str()))
    {
        display->print("Logger::logToSDCard() -> Failed to append to log file.");
    }
}
