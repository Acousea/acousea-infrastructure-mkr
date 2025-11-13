#include "WatchDogUtils.hpp"

#include "ClassName.h"
#include "Logger/Logger.h"

#ifdef ARDUINO_ARCH_SAMD
#include <Adafruit_SleepyDog.h>
#include <ArduinoLowPower.h>
#include <USB/USBAPI.h>
#define HAS_WATCHDOG 1
#else
#include <thread>
#include <chrono>
#define HAS_WATCHDOG 0
#endif

namespace WatchdogUtils
{
    CLASS_NAME(WatchdogUtils)

#if HAS_WATCHDOG
    static inline int lastTimeoutMs = -1; // valor por defecto razonable

    void enable(int timeoutMs)
    {
        if (timeoutMs >= 16000)
        {
            LOG_CLASS_WARNING("Watchdog timeout too high (%d ms). Setting to 16000 ms.", timeoutMs);
            timeoutMs = 16000;
        }

        lastTimeoutMs = timeoutMs; // <<--- GUARDARLO AQUÍ

        Watchdog.enable(lastTimeoutMs);
        LOG_CLASS_INFO("Watchdog enabled (%d ms)", timeoutMs);

        LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, []
        {
        }, CHANGE);
        LOG_CLASS_INFO("RTC Alarm wakeup interrupt attached.");

        logResetCause();
    }

    void reset()
    {
        LOG_CLASS_INFO("Watchdog reset() called.");
        Watchdog.reset();
    }

    void disable()
    {
        LOG_CLASS_INFO("Watchdog disable() called.");
        Watchdog.disable();
    }

    void sleepFor(const uint32_t ms)
    {
        (ms == 0)
            ? LOG_CLASS_WARNING("Sleeping indefinitely")
            : LOG_CLASS_WARNING("Sleeping for %lu ms", static_cast<unsigned long>(ms));

        Watchdog.reset();
        Watchdog.disable();
        USBDevice.detach();

        if (ms == 0) LowPower.sleep();
        else LowPower.sleep(ms);

        USBDevice.attach();
        Watchdog.enable(lastTimeoutMs); // re-enable watchdog on wake-up

        LOG_CLASS_INFO("Wakeup from sleep.");
    }

    void logResetCause()
    {
        const uint8_t cause = Watchdog.resetCause();
        const char* desc = nullptr;
        switch (cause)
        {
        case 0x01: desc = "Power-on";
            break;
        case 0x02: desc = "Brown-out 1.2V";
            break;
        case 0x04: desc = "Brown-out 3.3V";
            break;
        case 0x10: desc = "External reset";
            break;
        case 0x20: desc = "Watchdog reset";
            break;
        case 0x40: desc = "System request";
            break;
        default: desc = "Unknown";
            break;
        }
        LOG_CLASS_WARNING("Last Reset cause: %s (0x%02X)", desc, cause);
    }

    int getTimeout()
    {
        return lastTimeoutMs;
    }

#else
    // -------- PC / native / test implementation --------
    void enable(int /*timeoutMs*/)
    {
        LOG_CLASS_INFO("Cannot enable watchdog (native build)");
    }
    void reset()
    {
        LOG_CLASS_INFO("Watchdog reset called. Watchdog disabled (native build)");
    }
    void disable()
    {
        LOG_CLASS_INFO("Watchdog disable called (native build)");
    }
    void sleepFor(uint32_t ms)
    {
        LOG_CLASS_INFO("Simulated sleep for %lu ms", static_cast<unsigned long>(ms));
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    void logResetCause()
    {
        LOG_CLASS_WARNING("Watchdog resetCause called");
    }
#endif
}
