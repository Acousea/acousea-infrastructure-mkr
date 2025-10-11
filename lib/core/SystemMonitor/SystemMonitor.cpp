#include "SystemMonitor.h"

#ifdef ARDUINO

void SystemMonitor::init(const int timeoutMs)
{
    Watchdog.enable(timeoutMs);
    logResetCause();
}

void SystemMonitor::sync() const
{
    Watchdog.reset();
    if (!batteryController)
    {
        Logger::logWarning(getClassNameString() + " -> No battery Controller provided");
        return;
    }
    checkBatteryProtection();
}

void SystemMonitor::logResetCause()
{
    const uint8_t cause = Watchdog.resetCause();
    std::string msg = "[RESET_CAUSE] ";

    switch (cause)
    {
    case 0x01: msg += "Power-on reset";
        break;
    case 0x02: msg += "Brown-out 12 detector reset";
        break;
    case 0x04: msg += "Brown-out 33 detector reset";
        break;
    case 0x10: msg += "External reset (RESET pin)";
        break;
    case 0x20: msg += "Watchdog reset";
        break;
    case 0x40: msg += "System reset request";
        break;
    default: msg += "Unknown cause (0x" + std::to_string(cause) + ")";
        break;
    }

    Logger::logWarning(msg);
}

void SystemMonitor::checkBatteryProtection() const
{
    constexpr uint8_t LOW_BATTERY_THRESHOLD = 20, AGGRESSIVE_BATTERY_THRESHOLD = 15, CRITICAL_BATTERY_THRESHOLD = 10;

    const auto soc = batteryController->voltageSOC_rounded();
    const auto status = batteryController->status();

    if (soc > LOW_BATTERY_THRESHOLD && status != acousea_BatteryStatus::acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING)
    {
        Logger::logInfo(getClassNameString() + " -> Battery OK (" + std::to_string(soc) + "%).");
        return;
    }
    Logger::logWarning(getClassNameString() + " -> Low battery detected (" + std::to_string(soc) + "%).");

    if (soc <= CRITICAL_BATTERY_THRESHOLD)
    {
        Logger::logWarning(
            getClassNameString() + " -> CRITICAL battery level (" + std::to_string(soc) + "%). Sleeping indefinitely.");
        Watchdog.disable();
        LowPower.sleep(); // indefinite sleep
        return;
    }

    if (soc <= AGGRESSIVE_BATTERY_THRESHOLD)
    {
        constexpr uint32_t AGGRESIVE_BATTERY_SLEEP_MS = 600000;
        Logger::logWarning(
            getClassNameString() + " -> Aggressive protection mode (" + std::to_string(soc) +
            "%). Shutting down RockPi and sleeping.");
        if (rockpiController) rockpiController->commandShutdown();
        Watchdog.disable();
        LowPower.sleep(AGGRESIVE_BATTERY_SLEEP_MS); // 10 minutes rest before next possible check
        return;
    }

    if (soc <= LOW_BATTERY_THRESHOLD)
    {
        constexpr uint32_t LOW_BATTERY_SLEEP_MS = 300000;
        Logger::logWarning(
            getClassNameString() + " -> Low battery mode (" + std::to_string(soc) +
            "%). Sleeping 5 minutes, RockPi stays ON.");
        Watchdog.disable();
        LowPower.sleep(LOW_BATTERY_SLEEP_MS); // 5 minutes
        return;
    }

    Logger::logInfo(getClassNameString() + " -> Battery level normal (" + std::to_string(soc) + "%).");
}


#endif // ARDUINO
