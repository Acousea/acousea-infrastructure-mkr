#include "PMICBatteryController.h"

bool PMICBatteryController::init() {
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

    // Verificar si hubo algún failure durante la configuración
    return !error;
}

uint8_t PMICBatteryController::percentage() {
    // Warning: It's not possible to get the battery percentage from the PMIC
    return 0;
}

uint8_t PMICBatteryController::status() {
    return PMIC.chargeStatus();
}

void PMICBatteryController::printStatus() const {
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

void PMICBatteryController::applySettings() {
    error |= !PMIC.setInputCurrentLimit(INPUT_CURRENT_LIMIT) && printError("setInputCurrentLimit");
    error |= !PMIC.setInputVoltageLimit(INPUT_VOLTAGE_LIMIT) && printError("setInputVoltageLimit");
    error |= !PMIC.setMinimumSystemVoltage(MIN_SYSTEM_VOLTAGE) && printError("setMinimumSystemVoltage");
    error |= !PMIC.setChargeVoltage(CHARGE_VOLTAGE) && printError("setChargeVoltage");
    error |= !PMIC.setChargeCurrent(CHARGE_CURRENT) && printError("setChargeCurrent");
    error |= !PMIC.enableCharge() && printError("enableCharge");
}

bool PMICBatteryController::printError(const char *functionName) const {
    Serial.print("ERROR: ");
    Serial.print(functionName);
    Serial.println("() failed!");
    return true;
}

const char *PMICBatteryController::getChargeStatusMessage(uint8_t chargeStatus) const {
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
