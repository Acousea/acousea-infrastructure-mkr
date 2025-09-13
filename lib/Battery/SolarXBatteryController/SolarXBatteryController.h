#ifndef SOLARXBATTERYCONTROLLER_H
#define SOLARXBATTERYCONTROLLER_H


#ifdef ARDUINO

#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include "Adafruit_INA219.h"
#include "IBatteryController.h"
#include "Curves.h"


class SolarXBatteryController : public IBatteryController
{
    struct VoltageRange
    {
        float min;
        float max;
    };

private:
    std::vector<Adafruit_INA219> sensors;

private:
    static constexpr float PEUKERT_EXPONENT = 1.1f; // Exponente de Peukert típico para baterías de plomo-ácido
    // Capacidad nominal de la batería en Ah
    static constexpr float NOMINAL_CAPACITY_AH = 14.0f;
    // Número de celdas en serie
    static constexpr uint8_t NUM_CELLS = 6;
    // Voltaje mínimo y máximo de la batería
    static constexpr VoltageRange VOLTAGE_RANGE = {1.92f * NUM_CELLS, 2.15f * NUM_CELLS};
    // Curva lineal para mapear voltaje a porcentaje
    static const LinearCurve voltageToPercentageCurve;

    /** Calcula la capacidad efectiva (Ah) a la corriente actual
     * @param I Corriente de descarga (A)
     * @param Iref Corriente de referencia (A)
     * @param Cnom Capacidad nominal (Ah)
     * @param k Exponente de Peukert (típicamente entre 1.1 y 1.3)
     */
    float peukertCapacity(const float I, const float Iref, const float Cnom, const float k = PEUKERT_EXPONENT)
    {
        if (I <= 0.0) return Cnom; // Sin descarga -> nominal
        return Cnom * pow(Iref / I, k - 1);
    }

    /** Calcula el tiempo total de descarga estimado (horas) según Peukert
     * @param I Corriente de descarga (A)
     * @param Iref Corriente de referencia (A)
     * @param H Tiempo de descarga a Iref (horas)
     * @param k Exponente de Peukert (típicamente entre 1.1 y 1.3)
     */
    float peukertTime(const float I, const float Iref, const float H, const float k = PEUKERT_EXPONENT)
    {
        if (I <= 0.0) return INFINITY; // No hay descarga
        return H * pow(Iref / I, k);
    }

    static float remainingBatteryTime(const float stateOfChargePercentage,
                                      const float estimatedTimeForGivenCurrentAtFullCharge)
    {
        return stateOfChargePercentage * estimatedTimeForGivenCurrentAtFullCharge;
    }

public:
    SolarXBatteryController(const std::vector<uint8_t>& addresses);
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
