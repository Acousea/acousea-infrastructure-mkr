#include "SystemMonitor.h"

#ifdef ARDUINO

void SystemMonitor::init(const int timeoutMs)
{
    if (timeoutMs >= 16000)
    {
        Logger::logWarning(getClassNameString() + " -> Watchdog timeout too high (" + std::to_string(timeoutMs) +
            " ms). Setting to maximum allowed (16000 ms)."
        );
    }
    this->watchDogTimeoutMs = timeoutMs >= 16000 ? 16000 : timeoutMs;
    Watchdog.enable(watchDogTimeoutMs);
    Logger::logInfo(getClassNameString() + " -> Watchdog enabled with timeout " +
        std::to_string(watchDogTimeoutMs) + " ms."
    );
    logResetCause();
    // LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, rtcWakeup, CHANGE); // Dummy handler for wakeup
    LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, []
    {
    }, CHANGE); // Dummy handler for wakeup
    Logger::logInfo(getClassNameString() + " -> RTC Alarm wakeup interrupt attached.");
    if (!batteryController)
    {
        Logger::logWarning(getClassNameString() + " -> No battery Controller provided");
        return;
    }
    protectBattery();
}

void SystemMonitor::reset()
{
    Logger::logInfo(getClassNameString() + " -> reset()");
    Watchdog.reset();
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



void SystemMonitor::sleepFor(const uint32_t ms) const
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // LED ON mientras se ejecuta
    Logger::logWarning(getClassNameString() + " -> sleeping for " + (ms == 0 ? "indefinite" : std::to_string(ms) + " ms") + "...");
    Watchdog.reset();
    Watchdog.disable();
    USBDevice.detach();

    ms == 0 ? LowPower.sleep() : LowPower.sleep(ms); // indefinite sleep

    USBDevice.attach();
    Logger::logInfo(getClassNameString() + " -> woke up from " + (ms == 0 ? "indefinite" : std::to_string(ms) + " ms") + " sleep.");
    Watchdog.enable(watchDogTimeoutMs); // re-enable watchdog on wake-up
    digitalWrite(LED_BUILTIN, LOW); // LED OFF al despertar
}

void SystemMonitor::protectBattery() const
{
    if (!batteryController)
    {
        Logger::logWarning(getClassNameString() + " -> No battery Controller provided");
        return;
    }

    constexpr uint8_t
        HIGH_BATTERY_RESTART_THRESHOLD = 30,
        LOW_BATTERY_THRESHOLD = 20,
        AGGRESSIVE_BATTERY_THRESHOLD = 15,
        CRITICAL_BATTERY_THRESHOLD = 10;
    const auto soc = batteryController->voltageSOC_rounded();

    if (soc > HIGH_BATTERY_RESTART_THRESHOLD)
    {
        Logger::logInfo(getClassNameString() + " -> Battery level sufficient (" + std::to_string(soc) +
            "%). Starting RockPi if it was OFF.");
        if (rockpiController) rockpiController->commandStartup();
        return;
    }


    if (soc <= CRITICAL_BATTERY_THRESHOLD)
    {
        Logger::logWarning(getClassNameString() + " -> CRITICAL battery level (" + std::to_string(soc) +
            "%). Sleeping indefinitely. Shutting down RockPi.");
        if (rockpiController) rockpiController->commandShutdown();
        SystemMonitor::sleepFor(0); // indefinite sleep
        return;
    }

    if (soc <= AGGRESSIVE_BATTERY_THRESHOLD)
    {
        constexpr uint32_t AGGRESIVE_BATTERY_SLEEP_MS = 600000; // 10 minutes
        Logger::logWarning(getClassNameString() + " -> Aggressive protection mode (" + std::to_string(soc) +
            "%). Shutting down RockPi and sleeping.");
        if (rockpiController) rockpiController->commandShutdown();
        SystemMonitor::sleepFor(AGGRESIVE_BATTERY_SLEEP_MS);
        return;
    }

    if (soc <= LOW_BATTERY_THRESHOLD)
    {
        constexpr uint32_t LOW_BATTERY_SLEEP_MS = 60000; // 1 minute
        Logger::logWarning(getClassNameString() + " -> Low battery mode (" + std::to_string(soc) +
            "%). Sleeping 5 minutes, RockPi stays ON if it was already ON.");
        SystemMonitor::sleepFor(LOW_BATTERY_SLEEP_MS);
        return;
    }

    Logger::logInfo(getClassNameString() + " -> Battery level normal (" + std::to_string(soc) + "%).");
}


#endif // ARDUINO
