#include "SolarXBatteryController.h"

SolarXBatteryController::SolarXBatteryController(uint8_t address) {
    ina219 = Adafruit_INA219(address);
}

bool SolarXBatteryController::init() {
    if (!ina219.begin())
    {
        Serial.println("Error initializing sensor INA219");
        return false;
    }
    // Configurar el sensor a un rango específico si es necesario
    ina219.setCalibration_16V_400mA();
    return true;
}

uint8_t SolarXBatteryController::percentage() {
    float voltage = getVoltage();

    // Asegurarse de que el voltaje esté dentro del rango esperado
    if (voltage > MAX_VOLTAGE)
        voltage = MAX_VOLTAGE;
    if (voltage < MIN_VOLTAGE)
        voltage = MIN_VOLTAGE;

    // Calcular el porcentaje basado en el voltaje
    float percentage = ((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0;

    // Devolver el porcentaje en formato uint8_t
    return static_cast<uint8_t>(percentage);
}

uint8_t SolarXBatteryController::status() {
    float current = getCurrent();
    if (current > 0) return 1; // Cargando
    if (current == 0) return 2; // Desconectada
    return 0; // Estado desconocido
}

float SolarXBatteryController::getVoltage() {
    return ina219.getBusVoltage_V();
}

float SolarXBatteryController::getCurrent() {
    return ina219.getCurrent_mA() / 1000.0; // Convertir a amperios
}
