#ifdef ARDUINO

#include "SolarXBatteryController.h"

const LinearCurve SolarXBatteryController::voltageToPercentageCurve(
    (VOLTAGE_RANGE.max - VOLTAGE_RANGE.min) / 100,
    VOLTAGE_RANGE.min
);

SolarXBatteryController::SolarXBatteryController(const std::vector<uint8_t>& addresses)
{
    for (const uint8_t addr : addresses)
    {
        sensors.emplace_back(addr);
    }
}


bool SolarXBatteryController::init() {
    for (size_t i = 0; i < sensors.size(); i++) {
        if (!sensors[i].begin()) {
            Serial.print("Error inicializando INA219 en índice ");
            Serial.println(i);
            return false;
        }
        sensors[i].setCalibration_16V_400mA(); // Configuración para 16V y 400mA
    }
    return true;
}


uint8_t SolarXBatteryController::percentage()
{
    float voltage = getVoltage();

    // Clamp al rango válido
    if (voltage > VOLTAGE_RANGE.max)
        voltage = VOLTAGE_RANGE.max;
    if (voltage < VOLTAGE_RANGE.min)
        voltage = VOLTAGE_RANGE.min;

    // Usar curva lineal definida
    float percentage = voltageToPercentageCurve.forward(voltage);

    return static_cast<uint8_t>(percentage);
}

uint8_t SolarXBatteryController::status()
{
    float current = getCurrent();

    if (current > 0.05f) return 1;   // Cargando
    if (fabs(current) < 0.01f) return 2; // Desconectada (sin flujo)
    return 0; // Descargando o estado desconocido
}

float SolarXBatteryController::getVoltage()
{
    if (sensors.empty()) return 0.0f;

    float total = 0.0f;
    for (auto& s : sensors)
        total += s.getBusVoltage_V();

    return total / sensors.size(); // Promedio
}

float SolarXBatteryController::getCurrent()
{
    if (sensors.empty()) return 0.0f;

    float total = 0.0f;
    for (auto& s : sensors)
        total += s.getCurrent_mA() / 1000.0f; // mA -> A

    return total / sensors.size(); // Promedio
}


#endif
