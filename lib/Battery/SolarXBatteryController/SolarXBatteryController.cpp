#ifdef ARDUINO

#include "SolarXBatteryController.h"

#include <algorithm>
#include <vector>
#include <Arduino.h>
#include <Wire.h>
#include "Logger/Logger.h"
#include "time/getMillis.hpp"


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
    Wire.setTimeout(1500); // 1500 ms de timeout

    if (!batterySensor.begin()) {
        LOG_CLASS_ERROR("Error initializing battery INA219 at 0x%02X", batteryAddress);
        return false;
    }
    batterySensor.setCalibration_32V_1A();

    if (!panelSensor.begin()) {
        LOG_CLASS_ERROR("Error initializing panel INA219 at 0x%02X", panelAddress);
        return false;
    }
    panelSensor.setCalibration_32V_2A();

    _initialized = true;
    LOG_CLASS_INFO("Correctly initialized INA219 sensors at 0x%02X (battery) and 0x%02X (panel)",
                   batteryAddress, panelAddress);

    initialBatteryCalibration();
    return true;
}


void SolarXBatteryController::initialBatteryCalibration() {
    const auto currentA = batteryCurrentAmp();
    const auto voltageSOC = static_cast<float>(voltageSOC_accurate());

    if (fabs(currentA) < CURRENT_IDLE_THRESHOLD) {
        coulombCountedAh = (voltageSOC / 100.0f) * NOMINAL_CAPACITY_AH;
        cachedCombinedSOC = voltageSOC;
        LOG_CLASS_INFO("%sInitial SOC set from voltage: %.1f%%",
                 getClassNameCString(), voltageSOC);
    } else {
        coulombCountedAh = 0.5f * NOMINAL_CAPACITY_AH;
        cachedCombinedSOC = 50.0f;
        LOG_CLASS_WARNING("%sInitial SOC set to 50%% (battery not idle)",
                    getClassNameCString());
    }

    lastSyncTime = getMillis();
}

void SolarXBatteryController::sync() {
    if (!_initialized) {
        LOG_CLASS_INFO("Sensor not initialized. Call init() first.");
        return;
    }

    const unsigned long now = getMillis();
    const unsigned long delta_ms = now - lastSyncTime; // seguro incluso si millis() se desborda
    lastSyncTime = now;


    // --- Coulomb counting --- (clamp?)
    const double dt_h = static_cast<double>(delta_ms) / 3.6e6; // ms → h
    const float currentA = batteryCurrentAmp(); // + carga, - descarga
    coulombCountedAh += currentA * static_cast<float>(dt_h);
    coulombCountedAh = std::clamp(coulombCountedAh, 0.0f, NOMINAL_CAPACITY_AH);

    // --- Obtener SOCs individuales ---
    const auto coulombSOC = static_cast<float>(coulombSOC_accurate());
    const auto voltageSOC = static_cast<float>(voltageSOC_accurate());

    // --- Dynamic and combined SOC ---
    const float alpha = (fabs(currentA) > CURRENT_IDLE_THRESHOLD) ? ALPHA_SOC_MAX : ALPHA_SOC_MIN; // más peso al voltaje en reposo
    const float combinedSOC = (alpha * coulombSOC) +
                              ((1.0f - alpha) * voltageSOC);



    LOG_CLASS_INFO("sync() -> VoltageSOC=%.2f%%, CoulombSOC=%.2f%%, CombinedSOC=%.2f%%",
               voltageSOC, coulombSOC, combinedSOC);


    // Actualizar el estado interno
    cachedCombinedSOC = combinedSOC;
}

float SolarXBatteryController::combinedSOC_accurate() const {
    return cachedCombinedSOC;
}

uint8_t SolarXBatteryController::combinedSOC_rounded() const {
    return static_cast<uint8_t>(cachedCombinedSOC);
}


double SolarXBatteryController::coulombSOC_accurate() const {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0.0;
    }

    const double soc = (coulombCountedAh / NOMINAL_CAPACITY_AH) * 100.0;
    LOG_CLASS_INFO("coulombSOC_accurate() -> Coulomb SOC: %.2f%%", soc);

    return std::clamp(soc, 0.0, 100.0);
}

uint8_t SolarXBatteryController::coulombSOC_rounded() const {
    return static_cast<uint8_t>(coulombSOC_accurate());
}


double SolarXBatteryController::voltageSOC_accurate() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    float voltage = batteryVolts();

    LOG_CLASS_INFO("voltageSOC_accurate() -> Battery voltage: %.2f V", voltage);
    if (voltage > VOLTAGE_RANGE.max) voltage = VOLTAGE_RANGE.max;
    if (voltage < VOLTAGE_RANGE.min) voltage = VOLTAGE_RANGE.min;

    const double percentage = voltageToPercentageCurve.inverse(voltage);
    LOG_CLASS_INFO("voltageSOC_accurate() -> Voltage SOC: %.2f%%", percentage);
    return std::clamp(percentage, 0.0, 100.0);
}

uint8_t SolarXBatteryController::voltageSOC_rounded() {
    const double percentage = voltageSOC_accurate();
    return static_cast<uint8_t>(percentage);
}

acousea_BatteryStatus SolarXBatteryController::status() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return acousea_BatteryStatus_BATTERY_STATUS_ERROR;
    }
    const float current = batteryCurrentAmp();
    LOG_CLASS_INFO("status() -> Battery current: %.3f A", current);
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
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    const float batteryVoltage = batterySensor.getBusVoltage_V();
    LOG_CLASS_INFO("Battery voltage: %.2f V", batteryVoltage);
    return batteryVoltage;
}

float SolarXBatteryController::panelVolts() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    const float panelVoltage = panelSensor.getBusVoltage_V();
    LOG_CLASS_INFO("Panel voltage: %.2f V", panelVoltage);
    return panelVoltage;
}


float SolarXBatteryController::batteryCurrentAmp() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    const float batteryCurrentAmp = batterySensor.getCurrent_mA() / 1000.0f;
    LOG_CLASS_INFO("Battery current: %.3f A", batteryCurrentAmp);
    return batteryCurrentAmp;
}

float SolarXBatteryController::panelCurrentAmp() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    const float panelCurrentAmp = panelSensor.getCurrent_mA() / 1000.0f;
    LOG_CLASS_INFO("Panel current: %.3f A", panelCurrentAmp);
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
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
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

    LOG_CLASS_INFO("netCurrentConsumption() -> System current consumption: %.3f A (Panel=%.3f A, Battery=%.3f A)",
               balance, panel, batt);

    return balance;
}

// Potencia que entrega el panel al sistema (W)
float SolarXBatteryController::panelPowerWatts() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    return panelVolts() * panelCurrentAmp();
}

// Potencia que entrega o recibe la batería (W)
// Positiva = carga, Negativa = descarga
float SolarXBatteryController::batteryPowerWatts() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    return batteryVolts() * batteryCurrentAmp();
}


// Potencia neta consumida por el sistema (W)
// Igual a panelPower + batteryPower
float SolarXBatteryController::netPowerConsumptionWatts() {
    if (!_initialized) {
        LOG_CLASS_ERROR("Sensor not initialized. Call init() first.");
        return 0;
    }
    const float panelP = panelPowerWatts();
    const float battP = batteryPowerWatts();

    // Filter noise: below 0.1W is considered zero
    const float panelPFiltered = (panelP < 0.1f) ? 0.0f : panelP;
    const float battPFiltered = (fabs(battP) < 0.1f) ? 0.0f : battP;

    LOG_CLASS_INFO("Filtered Power readings: Panel=%.2f W, Battery=%.2f W",
               panelPFiltered, battPFiltered);

    // Calculate the net consumption of the system
    // If battery is charging (battP > 0), panel covers system + charges battery.
    // If battery is discharging (battP < 0), battery covers system deficit.
    // Result is always positive: net power drawn by the system.
    // In both cases, the formula is the same: (panelP - battP)
    const float netP = panelP - battP;


    LOG_CLASS_INFO("netPowerConsumptionWatts() -> System power consumption: %.2f W (Panel=%.2f W, Battery=%.2f W)",
               netP, panelP, battP);

    return netP;
}


#endif
