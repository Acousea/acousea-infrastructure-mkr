#ifdef ARDUINO

#include "SolarXBatteryController.h"

inline std::string toHexString(const uint8_t value){
    char buf[8];
    std::sprintf(buf, "%02X", value);
    return std::string(buf);
}

const LinearCurve SolarXBatteryController::voltageToPercentageCurve(
    (VOLTAGE_RANGE.max - VOLTAGE_RANGE.min) / 100.0f,
    VOLTAGE_RANGE.min
);

SolarXBatteryController::SolarXBatteryController(const uint8_t batteryAddr, const uint8_t panelAddr)
    : batteryAddress(batteryAddr),
      panelAddress(panelAddr),
      batterySensor(batteryAddr),
      panelSensor(panelAddr){
}

bool SolarXBatteryController::init(){
    if (!batterySensor.begin()){
        Logger::logError(getClassNameString() +
            "Error initializing battery INA219 at 0x" + toHexString(batteryAddress));
        return false;
    }
    batterySensor.setCalibration_16V_400mA();

    if (!panelSensor.begin()){
        Logger::logError(getClassNameString() +
            "Error initializing panel INA219 at 0x" + toHexString(panelAddress));
        return false;
    }
    panelSensor.setCalibration_16V_400mA();

    _initialized = true;
    Logger::logInfo(getClassNameString() +
        "Correctly initialized INA219 sensors at 0x" +
        toHexString(batteryAddress) + " (battery) and 0x" +
        toHexString(panelAddress) + " (panel)");
    return true;
}

uint8_t SolarXBatteryController::percentage(){
    if (!_initialized){
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.\n");
        return 0;
    }
    float voltage = readInternalVoltage();

    Logger::logInfo(getClassNameString() + "Voltage: " + std::to_string(voltage) + " V\n");

    if (voltage > VOLTAGE_RANGE.max) voltage = VOLTAGE_RANGE.max;
    if (voltage < VOLTAGE_RANGE.min) voltage = VOLTAGE_RANGE.min;

    const float percentage = voltageToPercentageCurve.inverse(voltage);
    Logger::logInfo(getClassNameString() + "Percentage (before clamp): " + std::to_string(percentage) + " %\n");

    return static_cast<uint8_t>(percentage);
}

uint8_t SolarXBatteryController::status(){
    if (!_initialized){
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.\n");
        return 0;
    }
    const float current = readInternalCurrent();
    Logger::logInfo("[SolarXBattery] Current: " + std::to_string(current) + " A\n");

    if (current > 0.05f) return 1; // Cargando
    if (fabs(current) < 0.01f) return 2; // Desconectada
    return 0; // Descargando
}

float SolarXBatteryController::readInternalVoltage(){
    if (!_initialized){
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.\n");
        return 0;
    }

    const float batteryVoltage = batterySensor.getBusVoltage_V();
    const float panelVoltage = panelSensor.getBusVoltage_V();

    Logger::logInfo(getClassNameString() + "Battery voltage: " + std::to_string(batteryVoltage) + " V\n");
    Logger::logInfo(getClassNameString() + "Panel voltage: " + std::to_string(panelVoltage) + " V\n");

    // aquí decides: devolver voltaje de batería o promedio
    return batteryVoltage;
}

float SolarXBatteryController::readInternalCurrent(){
    if (!_initialized){
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.\n");
        return 0;
    }

    const float batteryCurrentAmp = batterySensor.getCurrent_mA() / 1000.0f;
    const float panelCurrentAmp = panelSensor.getCurrent_mA() / 1000.0f;

    Logger::logInfo(getClassNameString() + "Battery current: " + std::to_string(batteryCurrentAmp) + " A\n");
    Logger::logInfo(getClassNameString() + "Panel current: " + std::to_string(panelCurrentAmp) + " A\n");

    // Aquí decides: ¿interesa el de batería, el del panel o ambos?
    return batteryCurrentAmp;
}

#endif
