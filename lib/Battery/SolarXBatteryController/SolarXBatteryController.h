#ifndef SOLARXBATTERYCONTROLLER_H
#define SOLARXBATTERYCONTROLLER_H


#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_INA219.h"
#include "IBatteryController.h"

class SolarXBatteryController : public IBatteryController
{
private:
    Adafruit_INA219 ina219;
    const int NUM_CELLS = 6;  // Número de celdas en la batería
    const float MAX_VOLTAGE = 2.15*NUM_CELLS;  // Voltaje máximo de la batería cargada
    const float MIN_VOLTAGE = 1.92*NUM_CELLS;  // Voltaje mínimo de la batería descargada

public:

    SolarXBatteryController(uint8_t address);

    // Inicializar el sensor INA219
    bool init() override;

    // Obtener el porcentaje estimado de batería
    uint8_t percentage() override;

    // Obtener el estado de la batería (0 = Desconocido, 1 = Cargando, 2 = Desconectada)
    uint8_t status() override;

    // Obtener el voltaje actual de la batería
    float getVoltage();

    // Obtener la corriente actual de la batería
    float getCurrent();
};

#endif // ARDUINO

#endif // AdafruitLC_MANAGER_H