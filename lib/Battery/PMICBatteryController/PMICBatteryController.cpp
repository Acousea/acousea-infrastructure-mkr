#include "PMICBatteryController.h"


bool PMICBatteryController::init() {
    if (!PMIC.begin()) {
        ErrorHandler::handleError("Failed to initialize PMIC!");
        return false;
    }

    // Apply the configuration
    applySettings();

    // Wait for a second before checking the status
    delay(1000);

    // Log the initial status
    printStatus();

    // Check if any errors occurred during configuration
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
    Logger::logInfo("Charge status: " + std::string(getChargeStatusMessage(PMIC.chargeStatus())));
    Logger::logInfo("Battery is connected: " + std::string(PMIC.isBattConnected() ? "Yes" : "No"));
    Logger::logInfo("Power is good: " + std::string(PMIC.isPowerGood() ? "Yes" : "No"));
    Logger::logInfo("Charge current (A): " + std::to_string(PMIC.getChargeCurrent()));
    Logger::logInfo("Charge voltage (V): " + std::to_string(PMIC.getChargeVoltage()));
    Logger::logInfo("Minimum system voltage (V): " + std::to_string(PMIC.getMinimumSystemVoltage()));
    Logger::logInfo(
        "Battery voltage is below minimum system voltage: " + std::string(PMIC.canRunOnBattery() ? "Yes" : "No"));
}

void PMICBatteryController::applySettings() {
    if (!PMIC.setInputCurrentLimit(INPUT_CURRENT_LIMIT)) {
        error = true;
        Logger::logError("PMIC.setInputCurrentLimit() failed!");
    }

    if (!PMIC.setInputVoltageLimit(INPUT_VOLTAGE_LIMIT)) {
        error = true;
        Logger::logError("PMIC.setInputVoltageLimit() failed!");
    }

    if (!PMIC.setMinimumSystemVoltage(MIN_SYSTEM_VOLTAGE)) {
        error = true;
        Logger::logError("PMIC.setMinimumSystemVoltage() failed!");
    }

    if (!PMIC.setChargeVoltage(CHARGE_VOLTAGE)) {
        error = true;
        Logger::logError("PMIC.setChargeVoltage() failed!");
    }

    if (!PMIC.setChargeCurrent(CHARGE_CURRENT)) {
        error = true;
        Logger::logError("PMIC.setChargeCurrent() failed!");
    }
    if (!PMIC.enableCharge()) {
        error = true;
        Logger::logError("PMIC.enableCharge() failed!");
    }
}


const char *PMICBatteryController::getChargeStatusMessage(uint8_t chargeStatus) const {
    switch (chargeStatus) {
        case NOT_CHARGING: return "Not charging";
        case PRE_CHARGING: return "Pre charging";
        case FAST_CHARGING: return "Fast charging";
        case CHARGE_TERMINATION_DONE: return "Charge termination done";
        default: return "Unknown status";
    }
}
