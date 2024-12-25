#include "LocationModule.h"

LocationModule LocationModule::from(float latitude, float longitude) {
    return LocationModule(latitude, longitude);
}

LocationModule LocationModule::from(const std::vector<uint8_t> &value) {
    if (value.size() < sizeof(float) * 2) {
        ErrorHandler::handleError("Invalid value size for LocationModule");
    }
    return LocationModule(value);
}

float LocationModule::getLatitude() const {
    return latitude;
}

float LocationModule::getLongitude() const {
    return longitude;
}

LocationModule::LocationModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::LOCATION, value) {
    if (VALUE.size() < sizeof(float) * 2) {
        ErrorHandler::handleError("Invalid value size for LocationModule");
    }
    std::memcpy(&latitude, VALUE.data(), sizeof(float));
    std::memcpy(&longitude, VALUE.data() + sizeof(float), sizeof(float));
}

LocationModule::LocationModule(float latitude, float longitude)
        : SerializableModule(ModuleCode::TYPES::LOCATION, serializeValues(latitude, longitude)),
          latitude(latitude), longitude(longitude) {}

std::vector<uint8_t> LocationModule::serializeValues(float latitude, float longitude) {
    std::vector<uint8_t> value(sizeof(float) * 2);
    std::memcpy(value.data(), &latitude, sizeof(float));
    std::memcpy(value.data() + sizeof(float), &longitude, sizeof(float));
    return value;
}
