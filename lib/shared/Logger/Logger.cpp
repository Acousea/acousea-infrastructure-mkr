#include "Logger.h"

#include <malloc.h>   // mallinfo
extern "C" char* sbrk(int incr);

int freeMemoryStackVsHeap() {
    char top;
    return &top - reinterpret_cast<char*>(sbrk(0));
}

void Logger::logFreeMemory(const std::string& prefix) {
    struct mallinfo mi = mallinfo();

    int freeMem = freeMemoryStackVsHeap();

    logInfo(prefix +
        " Free stack/heap gap: " + std::to_string(freeMem) + " bytes, " +
        "Heap total (arena): " + std::to_string(mi.arena) + " bytes, " +
        "Heap used: " + std::to_string(mi.uordblks) + " bytes, " +
        "Heap free (reusable): " + std::to_string(mi.fordblks) + " bytes"
    );
}


// void Logger::logFreeMemory(const std::string& prefix) {
//     logInfo(prefix + " Free memory: " + std::to_string(freeMemory()) + " bytes");
// }

void Logger::initialize(
    IDisplay* display, StorageManager* sdManager, RTCController* rtc, const char* logFilePath, Mode mode){
    Logger::display = display;
    Logger::storageManager = sdManager;
    Logger::rtc = rtc;
    Logger::logFilePath = logFilePath;
    Logger::mode = mode;
}

void Logger::logError(const std::string& errorMessage){
    display->setColor(IDisplay::Color::RED);
    log("ERROR", errorMessage);
}

void Logger::logWarning(const std::string& warningMessage){
    display->setColor(IDisplay::Color::ORANGE);
    log("WARNING", warningMessage);
}

void Logger::logInfo(const std::string& infoMessage){
    display->setColor(IDisplay::Color::DEFAULT);
    log("INFO", infoMessage);
}

void Logger::logfError(const char* fmt, ...) {
    display->setColor(IDisplay::Color::RED);

    char buffer[256]; // ajusta según tu RAM disponible
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    log("ERROR", buffer);
}

void Logger::logfWarning(const char* fmt, ...) {
    display->setColor(IDisplay::Color::ORANGE);

    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    log("WARNING", buffer);
}

void Logger::logfInfo(const char* fmt, ...) {
    display->setColor(IDisplay::Color::DEFAULT);

    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    log("INFO", buffer);
}



void Logger::log(const std::string& logType, const std::string& message){
    switch (mode){
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


void Logger::printLog(){
    if (mode == Mode::SDCard && storageManager){
        std::string content = storageManager->readFile(logFilePath);
        if (content.empty()){
            display->print("Logger::printLog() -> No logs available.");
            return;
        }
        display->print("Logger::printLog() -> Log Content:");
        display->print(content.c_str());
    }
    else{
        display->print("Logger::printLog() -> SD card logging not enabled.");
    }
}

bool Logger::clearLog(){
    if (mode == Mode::SDCard && storageManager){
        return storageManager->overwriteFile(logFilePath, "");
    }
    return false;
}

std::string Logger::vectorToHexString(const std::vector<unsigned char>& data){
    std::string result = "";
    for (const auto& byte : data){
        char buffer[3];
        sprintf(buffer, "%02X", byte);
        result += buffer;
    }
    return result;
}

std::string Logger::getTimestampString(){
    if (rtc){
        currentTime = rtc->getEpoch();
    }
    else{
        currentTime = time(nullptr);
    }
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));
    return std::string(buffer);
}

void Logger::logToSerial(const std::string& logType, const std::string& message){
    display->print(("[" + getTimestampString() + "] " + logType + ": " + message).c_str());
}

void Logger::logToSDCard(const std::string& logType, const std::string& message){
    if (!storageManager){
        display->print("Logger::logToSDCard() -> StorageManager not initialized.");
        return;
    }
    const std::string entry = getTimestampString() + "," + logType + "," + message + "\n";
    if (!storageManager->appendToFile(logFilePath, entry)){
        display->print("Logger::logToSDCard() -> Failed to append to log file.");
    }
}
