#include "ICListenStatus.h"


ICListenStatus::ICListenStatus(int unitStatus, int batteryStatus, double batteryPercentage, double temperature,
                               double humidity, std::time_t timestamp)
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_STATUS,
                             serializeValues(unitStatus, batteryStatus, batteryPercentage, temperature, humidity,
                                             timestamp)),
          unitStatus(unitStatus), batteryStatus(batteryStatus), batteryPercentage(batteryPercentage),
          temperature(temperature), humidity(humidity), timestamp(timestamp) {}

ICListenStatus ICListenStatus::createDefault() {
    return ICListenStatus(0, 0, 0.0, 0.0, 0.0, std::time(nullptr));
}

ICListenStatus::ICListenStatus(ICListenStatus &&other) noexcept: SerializableModule(ModuleCode::TYPES::ICLISTEN_STATUS,
                                                                     serializeValues(other.unitStatus, other.batteryStatus, other.batteryPercentage,
                                                                         other.temperature, other.humidity, other.timestamp)),
                                                                 unitStatus(other.unitStatus),
                                                                 batteryStatus(other.batteryStatus),
                                                                 batteryPercentage(other.batteryPercentage),
                                                                 temperature(other.temperature),
                                                                 humidity(other.humidity),
                                                                 timestamp(other.timestamp) {
}

ICListenStatus & ICListenStatus::operator=(ICListenStatus &&other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(std::move(other));
        const_cast<int &>(unitStatus) = other.unitStatus;
        const_cast<int &>(batteryStatus) = other.batteryStatus;
        const_cast<double &>(batteryPercentage) = other.batteryPercentage;
        const_cast<double &>(temperature) = other.temperature;
        const_cast<double &>(humidity) = other.humidity;
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
      timestamp(other.timestamp) {
}

ICListenStatus & ICListenStatus::operator=(const ICListenStatus &other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(other);
        const_cast<int &>(unitStatus) = other.unitStatus;
        const_cast<int &>(batteryStatus) = other.batteryStatus;
        const_cast<double &>(batteryPercentage) = other.batteryPercentage;
        const_cast<double &>(temperature) = other.temperature;
        const_cast<double &>(humidity) = other.humidity;
        const_cast<std::time_t &>(timestamp) = other.timestamp;
    }
    return *this;
}

std::vector<uint8_t>
ICListenStatus::serializeValues(int unitStatus, int batteryStatus, double batteryPercentage, double temperature,
                                double humidity, std::time_t timestamp) {
    std::vector<uint8_t> value;
    value.push_back(static_cast<uint8_t>(unitStatus));
    value.push_back(static_cast<uint8_t>(batteryStatus));
    value.insert(value.end(), reinterpret_cast<const uint8_t *>(&batteryPercentage),
                 reinterpret_cast<const uint8_t *>(&batteryPercentage) + sizeof(batteryPercentage));
    value.insert(value.end(), reinterpret_cast<const uint8_t *>(&temperature),
                 reinterpret_cast<const uint8_t *>(&temperature) + sizeof(temperature));
    value.insert(value.end(), reinterpret_cast<const uint8_t *>(&humidity),
                 reinterpret_cast<const uint8_t *>(&humidity) + sizeof(humidity));
    value.insert(value.end(), reinterpret_cast<const uint8_t *>(&timestamp),
                 reinterpret_cast<const uint8_t *>(&timestamp) + sizeof(timestamp));
    return value;
}

ICListenStatus ICListenStatus::fromBytes(const std::vector<uint8_t> &data) {
    if (data.size() < 25) {
        ErrorHandler::handleError("Invalid data size for ICListenStatus");
//        throw std::invalid_argument("Invalid data size for ICListenStatus");
    }

    int unitStatus = data[0];
    int batteryStatus = data[1];
    double batteryPercentage;
    std::memcpy(&batteryPercentage, &data[2], sizeof(double));

    double temperature;
    std::memcpy(&temperature, &data[10], sizeof(double));

    double humidity;
    std::memcpy(&humidity, &data[18], sizeof(double));

    std::time_t timestamp;
    std::memcpy(&timestamp, &data[26], sizeof(std::time_t));

    return {unitStatus, batteryStatus, batteryPercentage, temperature, humidity, timestamp};

}
