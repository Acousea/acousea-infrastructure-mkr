#include "WatchDogManager.h"

#ifdef ARDUINO

void WatchdogManager::init(const uint32_t timeoutMs) {
    Watchdog.enable(timeoutMs);
    logResetCause();
}

void WatchdogManager::reset() {
    Watchdog.reset();
}

void WatchdogManager::logResetCause() {
    const uint8_t cause = Watchdog.resetCause();
    std::string msg = "[RESET_CAUSE] ";

    switch (cause) {
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

#endif // ARDUINO