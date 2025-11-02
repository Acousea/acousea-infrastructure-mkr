#include <cstdint>
#include <cstdio>
#include <iostream>

#include "../../lib/Battery/SolarXBatteryController/Curves.h"
#include "../../.pio/libdeps/mkrgsm1400-test/MKRGSM/src/utility/GSMRootCerts.h"

int main(){
    std::cout << "Test main()\n";
    std::cout << "Amount of root certs: " << std::size(GSM_ROOT_CERTS) << "\n";
    std::cout << "Amount of root certs: " << GSM_NUM_ROOT_CERTS << "\n";
}

int test_main(){
    struct VoltageRange{
        float min;
        float max;
    };
    static constexpr uint8_t NUM_CELLS = 6;
    static constexpr VoltageRange VOLTAGE_RANGE = {1.92f * NUM_CELLS, 2.15f * NUM_CELLS};
    const LinearCurve voltageToPercentageCurve(
        (VOLTAGE_RANGE.max - VOLTAGE_RANGE.min) / 100.0f,
        VOLTAGE_RANGE.min
    );

    const float testVoltages[] = {11.52f, 12.0f, 12.3f, 13.5f}; // Voltajes de prueba
    for (float voltage : testVoltages){
        if (voltage > VOLTAGE_RANGE.max)
            voltage = VOLTAGE_RANGE.max;
        if (voltage < VOLTAGE_RANGE.min)
            voltage = VOLTAGE_RANGE.min;

        const double percentage = voltageToPercentageCurve.inverse(voltage);
        std::printf("Voltage: %.2f V -> Percentage: %.2f %%\n", voltage, percentage);
    }
}
