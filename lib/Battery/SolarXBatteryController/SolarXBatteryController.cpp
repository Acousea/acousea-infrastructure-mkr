#ifdef ARDUINO

#include "SolarXBatteryController.h"

inline std::string toHexString(const uint8_t value) {
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
      panelSensor(panelAddr) {
}

bool SolarXBatteryController::init() {
    if (!batterySensor.begin()) {
        Logger::logError(getClassNameString() +
                         "Error initializing battery INA219 at 0x" + toHexString(batteryAddress));
        return false;
    }
    batterySensor.setCalibration_32V_1A();

    if (!panelSensor.begin()) {
        Logger::logError(getClassNameString() +
                         "Error initializing panel INA219 at 0x" + toHexString(panelAddress));
        return false;
    }
    panelSensor.setCalibration_32V_2A();

    _initialized = true;
    Logger::logInfo(getClassNameString() +
                    "Correctly initialized INA219 sensors at 0x" +
                    toHexString(batteryAddress) + " (battery) and 0x" +
                    toHexString(panelAddress) + " (panel)");
    return true;
}

double SolarXBatteryController::accuratePercentage() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    float voltage = batteryVolts();

    Logger::logInfo(getClassNameString() + "Voltage: " + std::to_string(voltage) + " V");

    if (voltage > VOLTAGE_RANGE.max) voltage = VOLTAGE_RANGE.max;
    if (voltage < VOLTAGE_RANGE.min) voltage = VOLTAGE_RANGE.min;

    const double percentage = voltageToPercentageCurve.inverse(voltage);
    Logger::logInfo(getClassNameString() + "Percentage (before clamp): " + std::to_string(percentage) + " %");
    return (percentage < 0.0) ? 0.0 : (percentage > 100.0) ? 100.0 : percentage;
}

uint8_t SolarXBatteryController::percentage() {
    const double percentage = accuratePercentage();
    return static_cast<uint8_t>(percentage);
}

acousea_BatteryStatus SolarXBatteryController::status() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return acousea_BatteryStatus_BATTERY_STATUS_ERROR;
    }
    const float current = batteryCurrentAmp();
    Logger::logInfo("[SolarXBattery]::status() Current: " + std::to_string(current) + " A");

    if (current > 0.05f) {
        return acousea_BatteryStatus_BATTERY_STATUS_CHARGING;
    }
    if (current < -0.05f) {
        return acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING;
    }

    return acousea_BatteryStatus_BATTERY_STATUS_IDLE; // nuevo estado “ni carga ni descarga”
}

float SolarXBatteryController::batteryVolts() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    const float batteryVoltage = batterySensor.getBusVoltage_V();
    Logger::logInfo(getClassNameString() + "Battery voltage: " + std::to_string(batteryVoltage) + " V");
    return batteryVoltage;
}

float SolarXBatteryController::panelVolts() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    const float panelVoltage = panelSensor.getBusVoltage_V();
    Logger::logInfo(getClassNameString() + "Panel voltage: " + std::to_string(panelVoltage) + " V");
    return panelVoltage;
}

float SolarXBatteryController::balanceVolts() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    const float batt = batteryVolts();
    const float panel = panelVolts();
    const float balance = panel - batt;
    Logger::logInfo(getClassNameString() + "Voltage balance (panel - battery): " + std::to_string(balance) + " V");
    return balance;
}


float SolarXBatteryController::batteryCurrentAmp() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    const float batteryCurrentAmp = batterySensor.getCurrent_mA() / 1000.0f;
    Logger::logInfo(getClassNameString() + "Battery current: " + std::to_string(batteryCurrentAmp) + " A");
    return batteryCurrentAmp;
}

float SolarXBatteryController::panelCurrentAmp() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    const float panelCurrentAmp = panelSensor.getCurrent_mA() / 1000.0f;
    Logger::logInfo(getClassNameString() + "Panel current: " + std::to_string(panelCurrentAmp) + " A");
    return panelCurrentAmp;
}

/**
Returns the system's total current consumption (always >= 0). Computed as panel contribution minus battery current:
 - If batt > 0 (charging), panel covers system + charges battery.
 - If batt < 0 (discharging), battery covers system deficit.
Result is always positive: net current drawn by the system.
**/
float SolarXBatteryController::netCurrentConsumption() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    float batt = batteryCurrentAmp(); // + charging, - discharging
    float panel = panelCurrentAmp(); // >= 0, since panel only provides power

    // Filter noise: below 5mA is considered zero
    if (panel < 0.005f) panel = 0.0f;
    if (fabs(batt) < 0.005f) batt = 0.0f;

    // Consider 2 cases:
    // 1. If battery is charging (batt > 0), then the system is consuming (panel - batt)
    // 2. If battery is discharging (batt < 0), then the system is consuming (panel + abs(batt)) = (panel - batt)
    // In both cases, the formula is the same: (panel - batt)

    const float balance = panel - batt; // since batt is negative when discharging

    Logger::logInfo(getClassNameString() + "System absolute current consumption " + std::to_string(balance) + " A");
    return balance;
}

// Potencia que entrega el panel al sistema (W)
float SolarXBatteryController::panelPowerWatts() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    return panelVolts() * panelCurrentAmp();
}

// Potencia que entrega o recibe la batería (W)
// Positiva = carga, Negativa = descarga
float SolarXBatteryController::batteryPowerWatts() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    return batteryVolts() * batteryCurrentAmp();
}


// Potencia neta consumida por el sistema (W)
// Igual a panelPower + batteryPower
float SolarXBatteryController::netPowerConsumptionWatts() {
    if (!_initialized) {
        Logger::logError(getClassNameString() + "Sensor not initialized. Call init() first.");
        return 0;
    }
    const float panelP = panelPowerWatts();
    const float battP = batteryPowerWatts();

    // Filter noise: below 0.1W is considered zero
    const float panelPFiltered = (panelP < 0.1f) ? 0.0f : panelP;
    const float battPFiltered = (fabs(battP) < 0.1f) ? 0.0f : battP;
    Logger::logInfo(getClassNameString() +
                    "Filtered Power readings: Panel=" + std::to_string(panelPFiltered) + " W, Battery=" +
                    std::to_string(battPFiltered) + " W");


    // Calculate the net consumption of the system
    // If battery is charging (battP > 0), panel covers system + charges battery.
    // If battery is discharging (battP < 0), battery covers system deficit.
    // Result is always positive: net power drawn by the system.
    // In both cases, the formula is the same: (panelP - battP)
    const float netP = panelP - battP;

    Logger::logInfo(getClassNameString() +
                    "System power consumption: " + std::to_string(netP) + " W "
                    "(Panel=" + std::to_string(panelP) + " W, Battery=" + std::to_string(battP) + " W)");
    return netP;
}


#endif
