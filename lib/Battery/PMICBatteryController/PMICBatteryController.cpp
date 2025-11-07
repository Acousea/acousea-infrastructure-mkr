#ifdef PLATFORM_ARDUINO

#include "PMICBatteryController.h"

#include "Arduino_PMIC.h"
#include <ErrorHandler/ErrorHandler.h>
#include <Logger/Logger.h>

bool PMICBatteryController::init() {
    if (!PMIC.begin()) {
        ERROR_HANDLE_CLASS("Failed to initialize PMIC!");
        return false;
    }

    // Apply the configuration
    applySettings();

    // Wait for a second before checking the status
    delay(1000);

    // Log the initial status
    printStatus();

    // Check if any errors occurred during configuration
    return !errorState;
}

bool PMICBatteryController::sync()
{
    return !errorState;
}

uint8_t PMICBatteryController::voltageSOC_rounded() {
    // Warning: It's not possible to get the battery percentage from the PMIC
    return 0;
}


acousea_BatteryStatus PMICBatteryController::status() {
    switch (PMIC.chargeStatus()) {
    case NOT_CHARGING: return acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING;
    case PRE_CHARGING: return acousea_BatteryStatus_BATTERY_STATUS_CHARGING;
    case FAST_CHARGING: return acousea_BatteryStatus_BATTERY_STATUS_CHARGING;
    case CHARGE_TERMINATION_DONE: return acousea_BatteryStatus_BATTERY_STATUS_FULL;
    default: return acousea_BatteryStatus_BATTERY_STATUS_ERROR;
    }
}


void PMICBatteryController::printStatus() const {
    LOG_CLASS_INFO("Charge status: %s", getChargeStatusMessage(PMIC.chargeStatus()));
    LOG_CLASS_INFO("Battery is connected: %s", PMIC.isBattConnected() ? "Yes" : "No");
    LOG_CLASS_INFO("Power is good: %s", PMIC.isPowerGood() ? "Yes" : "No");
    LOG_CLASS_INFO("Charge current (A): %.2f", PMIC.getChargeCurrent());
    LOG_CLASS_INFO("Charge voltage (V): %.2f", PMIC.getChargeVoltage());
    LOG_CLASS_INFO("Minimum system voltage (V): %.2f", PMIC.getMinimumSystemVoltage());
    LOG_CLASS_INFO("Battery voltage is below minimum system voltage: %s",
                     PMIC.canRunOnBattery() ? "Yes" : "No");
}


void PMICBatteryController::applySettings() {
    if (!PMIC.setInputCurrentLimit(INPUT_CURRENT_LIMIT)) {
        errorState = true;
        LOG_CLASS_ERROR("PMIC.setInputCurrentLimit() failed!");
    }

    if (!PMIC.setInputVoltageLimit(INPUT_VOLTAGE_LIMIT)) {
        errorState = true;
        LOG_CLASS_ERROR("PMIC.setInputVoltageLimit() failed!");
    }

    if (!PMIC.setMinimumSystemVoltage(MIN_SYSTEM_VOLTAGE)) {
        errorState = true;
        LOG_CLASS_ERROR("PMIC.setMinimumSystemVoltage() failed!");
    }

    if (!PMIC.setChargeVoltage(CHARGE_VOLTAGE)) {
        errorState = true;
        LOG_CLASS_ERROR("PMIC.setChargeVoltage() failed!");
    }

    if (!PMIC.setChargeCurrent(CHARGE_CURRENT)) {
        errorState = true;
        LOG_CLASS_ERROR("PMIC.setChargeCurrent() failed!");
    }
    if (!PMIC.enableCharge()) {
        errorState = true;
        LOG_CLASS_ERROR("PMIC.enableCharge() failed!");
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

#endif // ARDUINO