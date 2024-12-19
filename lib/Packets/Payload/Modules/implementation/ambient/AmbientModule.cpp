#include "AmbientModule.h"

AmbientModule AmbientModule::from(int temperature, int humidity) {
    return {temperature, humidity};
}

AmbientModule AmbientModule::from(const std::vector<uint8_t> &value) {
    if (value.size() < 2) {
        ErrorHandler::handleError("Invalid value size for AmbientModule");
    }
    return AmbientModule(value);
}

int AmbientModule::getTemperature() const {
    return temperature;
}

int AmbientModule::getHumidity() const {
    return humidity;
}

AmbientModule::AmbientModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::AMBIENT, value) {

    if (VALUE.size() < 2) {
        ErrorHandler::handleError("Invalid value size for AmbientModule");
    }
    temperature = VALUE[0];
    humidity = VALUE[1];
}

AmbientModule::AmbientModule(int temperature, int humidity)
        : SerializableModule(ModuleCode::TYPES::AMBIENT, {static_cast<uint8_t>(temperature), static_cast<uint8_t>(humidity)}) {
    this->temperature = temperature;
    this->humidity = humidity;
}
