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

ICListenStatus ICListenStatus::from(const std::vector<uint8_t> &data) {
    if (data.size() < 25) {
        ErrorHandler::handleError("Invalid data size for ICListenStatus");
//        throw std::invalid_argument("Invalid data size for ICListenStatus");
    }

    int unitStatus = data[0];
    int batteryStatus = data[1];
    double batteryPercentage = *reinterpret_cast<const double *>(&data[2]);
    double temperature = *reinterpret_cast<const double *>(&data[10]);
    double humidity = *reinterpret_cast<const double *>(&data[18]);
    std::time_t timestamp = *reinterpret_cast<const std::time_t *>(&data[26]);

    return {unitStatus, batteryStatus, batteryPercentage, temperature, humidity, timestamp};

}
