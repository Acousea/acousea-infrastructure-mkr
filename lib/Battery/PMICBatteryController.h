#ifndef PMIC_MANAGER_H
#define PMIC_MANAGER_H

#include <Arduino.h>
#include <Arduino_PMIC.h>
#include "IBattery.h"

class PMICBatteryController : public IBattery {
public:
    bool init() override {
        if (!PMIC.begin()) {
            Serial.println("ERROR: Failed to initialize PMIC!");
            return false;
        }

        // Aplicar la configuración
        applySettings();

        // Esperar un segundo antes de verificar el estado
        delay(1000);

        // Imprimir el estado inicial
        printStatus();

        // Verificar si hubo algún error durante la configuración
        return !error;
    }

    uint8_t percentage() override {
        // Warning: It's not possible to get the battery percentage from the PMIC
        return 0;
    }

    void printStatus() const {
        Serial.print("Charge status: ");
        Serial.println(getChargeStatusMessage(PMIC.chargeStatus()));

        Serial.print("Battery is connected: ");
        Serial.println(PMIC.isBattConnected() ? "Yes" : "No");

        Serial.print("Power is good: ");
        Serial.println(PMIC.isPowerGood() ? "Yes" : "No");

        Serial.print("Charge current (A): ");
        Serial.println(PMIC.getChargeCurrent(), 2);

        Serial.print("Charge voltage (V): ");
        Serial.println(PMIC.getChargeVoltage(), 2);

        Serial.print("Minimum system voltage (V): ");
        Serial.println(PMIC.getMinimumSystemVoltage(), 2);

        Serial.print("Battery voltage is below minimum system voltage: ");
        Serial.println(PMIC.canRunOnBattery() ? "Yes" : "No");
    }

private:
    bool error = false;

    static constexpr float INPUT_CURRENT_LIMIT = 2.0;
    static constexpr float INPUT_VOLTAGE_LIMIT = 3.88;
    static constexpr float MIN_SYSTEM_VOLTAGE = 3.5;
    static constexpr float CHARGE_VOLTAGE = 4.2;
    static constexpr float CHARGE_CURRENT = 0.375;

    void applySettings() {
        error |= !PMIC.setInputCurrentLimit(INPUT_CURRENT_LIMIT) && printError("setInputCurrentLimit");
        error |= !PMIC.setInputVoltageLimit(INPUT_VOLTAGE_LIMIT) && printError("setInputVoltageLimit");
        error |= !PMIC.setMinimumSystemVoltage(MIN_SYSTEM_VOLTAGE) && printError("setMinimumSystemVoltage");
        error |= !PMIC.setChargeVoltage(CHARGE_VOLTAGE) && printError("setChargeVoltage");
        error |= !PMIC.setChargeCurrent(CHARGE_CURRENT) && printError("setChargeCurrent");
        error |= !PMIC.enableCharge() && printError("enableCharge");
    }

    bool printError(const char* functionName) const {
        Serial.print("ERROR: ");
        Serial.print(functionName);
        Serial.println("() failed!");
        return true;
    }

    const char* getChargeStatusMessage(uint8_t chargeStatus) const {
        switch (chargeStatus) {
            case NOT_CHARGING:
                return "Not charging";
            case PRE_CHARGING:
                return "Pre charging";
            case FAST_CHARGING:
                return "Fast charging";
            case CHARGE_TERMINATION_DONE:
                return "Charge termination done";
            default:
                return "Unknown status";
        }
    }
};

#endif // PMIC_MANAGER_H
