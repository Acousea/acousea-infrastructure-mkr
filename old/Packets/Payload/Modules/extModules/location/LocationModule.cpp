#include "LocationModule.h"

LocationModule LocationModule::from(float latitude, float longitude) {
    return LocationModule(latitude, longitude);
}

LocationModule LocationModule::from(const std::vector<uint8_t> &value) {
    if (value.size() < sizeof(float) * 2) {
        ErrorHandler::handleError("Invalid value size for LocationModule");
    }

    // Convert from little endian to big endian
    float latitude, longitude;
    std::memcpy(&latitude, value.data(), sizeof(float));
    std::memcpy(&longitude, value.data() + sizeof(float), sizeof(float));

    // Reverse the byte order of the floats
    char *latitudeBytes = reinterpret_cast<char *>(&latitude);
    std::reverse(latitudeBytes, latitudeBytes + sizeof(float));
    char *longitudeBytes = reinterpret_cast<char *>(&longitude);
    std::reverse(longitudeBytes, longitudeBytes + sizeof(float));

    return LocationModule(latitude, longitude);
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

    // Convert from little endian to big endian
    std::memcpy(&latitude, value.data(), sizeof(float));
    std::memcpy(&longitude, value.data() + sizeof(float), sizeof(float));

    // Reverse the byte order of the floats
    std::reverse(reinterpret_cast<char *>(&latitude), reinterpret_cast<char *>(&latitude) + sizeof(float));
    std::reverse(reinterpret_cast<char *>(&longitude), reinterpret_cast<char *>(&longitude) + sizeof(float));
}

LocationModule::LocationModule(float latitude, float longitude)
    : SerializableModule(ModuleCode::TYPES::LOCATION, serializeValues(latitude, longitude)),
      latitude(latitude), longitude(longitude) {
}

std::vector<uint8_t> LocationModule::serializeValues(float latitude, float longitude) {
    std::vector<uint8_t> value(sizeof(float) * 2);

    // Ensure big endian encoding
    const auto latitudeBytes = reinterpret_cast<char *>(&latitude);
    std::reverse(latitudeBytes, latitudeBytes + sizeof(float));
    const auto longitudeBytes = reinterpret_cast<char *>(&longitude);
    std::reverse(longitudeBytes, longitudeBytes + sizeof(float));

    std::memcpy(value.data(), &latitude, sizeof(float));
    std::memcpy(value.data() + sizeof(float), &longitude, sizeof(float));
    return value;
}
