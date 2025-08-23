#include "RTCModule.h"

RTCModule RTCModule::from(std::time_t currentTime) {
    return RTCModule(currentTime);
}

RTCModule RTCModule::from(const std::vector<uint8_t> &value) {
    if (value.size() < sizeof(std::time_t)) {
        ErrorHandler::handleError("Invalid value size for RTCModule");
    }

    // Convert from little endian to big endian
    std::time_t currentTime;
    std::memcpy(&currentTime, value.data(), sizeof(std::time_t));

    // Reverse the byte order (assuming little endian system)
    char *currentTimeBytes = reinterpret_cast<char*>(&currentTime);
    std::reverse(currentTimeBytes, currentTimeBytes + sizeof(std::time_t));

    return RTCModule(currentTime);
}

std::time_t RTCModule::getCurrentTime() const {
    return currentTime;
}

RTCModule::RTCModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::REAL_TIME_CLOCK, value) {
    if (VALUE.size() < sizeof(std::time_t)) {
        ErrorHandler::handleError("Invalid value size for RTCModule");
    }

    // Handle potential endianness difference
    std::time_t currentTime;
    std::memcpy(&currentTime, VALUE.data(), sizeof(std::time_t));

    // Reverse the byte order of the time_t value
    char *currentTimeBytes = reinterpret_cast<char*>(&currentTime);
    std::reverse(currentTimeBytes, currentTimeBytes + sizeof(std::time_t));

    this->currentTime = currentTime;
}

RTCModule::RTCModule(std::time_t currentTime)
        : SerializableModule(ModuleCode::TYPES::REAL_TIME_CLOCK, serializeValue(currentTime)),
          currentTime(currentTime) {}

std::vector<uint8_t> RTCModule::serializeValue(std::time_t currentTime) {
    std::vector<uint8_t> value(sizeof(std::time_t));

    // Ensure big endian encoding
    char *currentTimeBytes = reinterpret_cast<char*>(&currentTime);
    std::reverse(currentTimeBytes, currentTimeBytes + sizeof(std::time_t));

    std::memcpy(value.data(), &currentTime, sizeof(std::time_t));
    return value;
}