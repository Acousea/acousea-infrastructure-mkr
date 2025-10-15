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
    Logger::logWarning(
        getClassNameString() + " -> sleeping for " + (ms == 0 ? "indefinite" : std::to_string(ms) + " ms") + "...");
    Watchdog.reset();
    Watchdog.disable();
    USBDevice.detach();

    ms == 0 ? LowPower.sleep() : LowPower.sleep(ms); // indefinite sleep

    USBDevice.attach();
    Logger::logInfo(
        getClassNameString() + " -> woke up from " + (ms == 0 ? "indefinite" : std::to_string(ms) + " ms") + " sleep.");
    Watchdog.enable(watchDogTimeoutMs); // re-enable watchdog on wake-up
    digitalWrite(LED_BUILTIN, LOW); // LED OFF al despertar
}

void SystemMonitor::manageRockPiAction(const unsigned long cooldownMs,
                                       void (RockPiPowerController::*action)() const) const
{
    const unsigned long now = getMillis();
    static unsigned long lastActionTime = now - cooldownMs - 1;

    if (now - lastActionTime < cooldownMs)
    {
        Logger::logWarning(
            getClassNameString() + "manageRockPiAction() -> Cooldown active (" + std::to_string(now - lastActionTime) +
            "/" + std::to_string(cooldownMs) + " ms)"
        );
        return;
    }

    if (!rockpiController)
    {
        Logger::logError(getClassNameString() + "manageRockPiAction() -> No RockPi controller available.");
        return;
    }

    (rockpiController->*action)();
    lastActionTime = now;
}


void SystemMonitor::protectBattery() const
{
    if (!batteryController)
    {
        Logger::logWarning(getClassNameString() + " -> No battery controller provided.");
        return;
    }
    const uint8_t soc = batteryController->voltageSOC_rounded();

    using Comparator = bool (*)(uint8_t, uint8_t);
    struct BatteryRule
    {
        uint8_t threshold;
        Comparator cmp;
        void (RockPiPowerController::*action)() const;
        uint32_t actionCooldownMs;
        int32_t sleepMs;
    };

    constexpr Comparator lessEq = +[](uint8_t a, uint8_t b) { return a <= b; };
    constexpr Comparator greater = +[](uint8_t a, uint8_t b) { return a > b; };
    constexpr BatteryRule rules[] = {
        // Critical level: shut down immediately and sleep indefinitely
        {10, lessEq, &RockPiPowerController::commandShutdown, 120000, 0},

        // Aggressive protection: shut down and sleep for 10 minutes
        {15, lessEq, &RockPiPowerController::commandShutdown, 120000, 600000},

        // Low battery: keep RockPi on but sleep for 5 minutes
        {20, lessEq, nullptr, 120000, 300000},

        // Battery sufficient: start RockPi if it is off, no sleep
        {30, greater, &RockPiPowerController::commandStartup, 120000, -1}
    };

    for (const auto& rule : rules)
    {
        if (rule.cmp(soc, rule.threshold))
        {
            Logger::logInfo(getClassNameString() + std::string(" -> Battery rule triggered (SOC=")
                + std::to_string(soc) + "%, threshold=" + std::to_string(rule.threshold) + ")");


            if (rockpiController && rule.action) manageRockPiAction(rule.actionCooldownMs, rule.action);
            if (rule.sleepMs >= 0) SystemMonitor::sleepFor(rule.sleepMs);
        }
    }

    Logger::logInfo(getClassNameString() + " -> Battery normal (" + std::to_string(soc) + "%).");
}


#endif // ARDUINO
