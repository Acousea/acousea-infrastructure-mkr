#include "ICListenStatus.h"

ICListenStatus::ICListenStatus(int unitStatus, int batteryStatus, float batteryPercentage, float temperature,
                               float humidity, std::time_t timestamp)
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_STATUS,
                             serializeValues(unitStatus, batteryStatus, batteryPercentage, temperature, humidity,
                                             timestamp)),
          unitStatus(unitStatus), batteryStatus(batteryStatus), batteryPercentage(batteryPercentage),
          temperature(temperature), humidity(humidity), timestamp(timestamp) {}

ICListenStatus ICListenStatus::createDefault() {
    return ICListenStatus(0, 0, 0.0f, 0.0f, 0.0f, std::time(nullptr));
}

ICListenStatus::ICListenStatus(ICListenStatus &&other) noexcept
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_STATUS,
                             serializeValues(other.unitStatus, other.batteryStatus, other.batteryPercentage,
                                            other.temperature, other.humidity, other.timestamp)),
          unitStatus(other.unitStatus),
          batteryStatus(other.batteryStatus),
          batteryPercentage(other.batteryPercentage),
          temperature(other.temperature),
          humidity(other.humidity),
          timestamp(other.timestamp) {}

ICListenStatus &ICListenStatus::operator=(ICListenStatus &&other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(std::move(other));
        const_cast<int &>(unitStatus) = other.unitStatus;
        const_cast<int &>(batteryStatus) = other.batteryStatus;
        const_cast<float &>(batteryPercentage) = other.batteryPercentage;
        const_cast<float &>(temperature) = other.temperature;
        const_cast<float &>(humidity) = other.humidity;
        const_cast<std::time_t &>(timestamp) = other.timestamp;
    }
    return *this;
}

ICListenStatus::ICListenStatus(const ICListenStatus &other) noexcept
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_STATUS,
                             serializeValues(other.unitStatus, other.batteryStatus, other.batteryPercentage,
                                            other.temperature, other.humidity, other.timestamp)),
          unitStatus(other.unitStatus),
          batteryStatus(other.batteryStatus),
          batteryPercentage(other.batteryPercentage),
          temperature(other.temperature),
          humidity(other.humidity),
          timestamp(other.timestamp) {}

ICListenStatus &ICListenStatus::operator=(const ICListenStatus &other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(other);
        const_cast<int &>(unitStatus) = other.unitStatus;
        const_cast<int &>(batteryStatus) = other.batteryStatus;
        const_cast<float &>(batteryPercentage) = other.batteryPercentage;
        const_cast<float &>(temperature) = other.temperature;
        const_cast<float &>(humidity) = other.humidity;
        const_cast<std::time_t &>(timestamp) = other.timestamp;
    }
    return *this;
}

std::vector<uint8_t>
ICListenStatus::serializeValues(int unitStatus, int batteryStatus, float batteryPercentage, float temperature,
                                float humidity, std::time_t timestamp) {
    std::vector<uint8_t> value;
    value.push_back(static_cast<uint8_t>(unitStatus));
    value.push_back(static_cast<uint8_t>(batteryStatus));
    value.push_back(static_cast<uint8_t>(std::clamp(static_cast<int>(batteryPercentage), 0, 255)));
    value.push_back(static_cast<int8_t>(std::clamp(static_cast<int>(temperature), -128, 127))); // Signed byte for temperature
    value.push_back(static_cast<uint8_t>(std::clamp(static_cast<int>(humidity), 0, 255)));
    value.insert(value.end(), reinterpret_cast<const uint8_t *>(&timestamp),
                 reinterpret_cast<const uint8_t *>(&timestamp) + sizeof(timestamp));
    return value;
}

ICListenStatus ICListenStatus::fromBytes(const std::vector<uint8_t> &data) {
    if (data.size() < 7) { // Adjusted for 1-byte floats
        ErrorHandler::handleError("Invalid data size for ICListenStatus");
    }

    int unitStatus = data[0];
    int batteryStatus = data[1];
    float batteryPercentage = static_cast<float>(data[2]);
    float temperature = static_cast<float>(static_cast<int8_t>(data[3])); // Signed byte for temperature
    float humidity = static_cast<float>(data[4]);

    std::time_t timestamp;
    std::memcpy(&timestamp, &data[5], sizeof(std::time_t));

    return {unitStatus, batteryStatus, batteryPercentage, temperature, humidity, timestamp};
}

std::string ICListenStatus::toString() const {
    return "ICListenStatus { unitStatus: " + std::to_string(unitStatus) +
           ", batteryStatus: " + std::to_string(batteryStatus) +
           ", batteryPercentage: " + std::to_string(batteryPercentage) +
           ", temperature: " + std::to_string(temperature) +
           ", humidity: " + std::to_string(humidity) +
           ", timestamp: " + std::to_string(timestamp) + " }";
}
