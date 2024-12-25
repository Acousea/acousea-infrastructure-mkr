#include "BatteryModule.h"

BatteryModule BatteryModule::from(uint8_t batteryPercentage, uint8_t batteryStatus) {
    return {batteryPercentage, batteryStatus};
}

BatteryModule BatteryModule::from(const std::vector<uint8_t> &value) {
    if (value.size() < 2) {
        ErrorHandler::handleError("Invalid value size for BatteryModule");
    }
    return BatteryModule(value);
}

uint8_t BatteryModule::getBatteryPercentage() const {
    return batteryPercentage;
}

uint8_t BatteryModule::getBatteryStatus() const {
    return batteryStatus;
}

BatteryModule::BatteryModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::BATTERY, value) {
    if (VALUE.size() < 2) {
        ErrorHandler::handleError("Invalid value size for BatteryModule");
    }
    batteryPercentage = VALUE[0];
    batteryStatus = VALUE[1];
}

BatteryModule::BatteryModule(uint8_t batteryPercentage, uint8_t batteryStatus)
        : SerializableModule(ModuleCode::TYPES::BATTERY, {batteryPercentage, batteryStatus}),
          batteryPercentage(batteryPercentage), batteryStatus(batteryStatus) {}
