#ifdef PLATFORM_ARDUINO

#include "BatteryProtectionPolicy.hpp"
#include "WatchDog/WatchDogUtils.hpp"
#include "time/getMillis.hpp"
#include "Logger/Logger.h"
#include <cstdint>

namespace
{
    using Comparator = bool (*)(std::uint8_t, uint8_t);

    struct BatteryRule
    {
        uint8_t threshold;
        Comparator cmp;
        void (PiController::*action)() const;
        uint32_t actionCooldownMs;
        int32_t sleepMs;
    };

    constexpr Comparator lessEq = +[](uint8_t a, uint8_t b) { return a <= b; };
    constexpr Comparator greater = +[](uint8_t a, uint8_t b) { return a > b; };

    constexpr BatteryRule rules[] = {
        {10, lessEq, &PiController::commandShutdown, 120000, 0},
        {15, lessEq, &PiController::commandShutdown, 120000, 600000},
        {20, lessEq, nullptr,                           120000, 300000},
        {30, greater, &PiController::commandStartup,    120000, -1}
    };
}

void BatteryProtectionPolicy::enforce() const
{
    // Try to sync battery data
    if (const bool syncOk = batteryCtrl.sync(); !syncOk)
    {
        LOG_CLASS_WARNING("BatteryProtectionPolicy -> Battery sync failed.");
        return;
    }

    const uint8_t soc = batteryCtrl.voltageSOC_rounded();

    for (const auto& rule : rules)
    {
        if (rule.cmp(soc, rule.threshold))
        {
            LOG_CLASS_INFO("BatteryProtectionPolicy -> Rule triggered (SOC=%u%%, threshold=%u)", soc, rule.threshold);

            static unsigned long lastActionTime = 0;
            const unsigned long now = getMillis();

            if (rule.action && (now - lastActionTime >= rule.actionCooldownMs))
            {
                (piCtrl.*rule.action)();
                lastActionTime = now;
            }

            // Portable sleep logic handled by WatchdogUtils
            if (rule.sleepMs >= 0)
            {
                if (rule.sleepMs == 0)
                    LOG_CLASS_WARNING(" -> Entering indefinite sleep due to critical battery");
                else
                    LOG_CLASS_WARNING(" -> Sleeping for %ld ms due to low battery", rule.sleepMs);

                WatchdogUtils::sleepFor(static_cast<uint32_t>(rule.sleepMs));
            }
            return;
        }
    }

    LOG_CLASS_INFO("BatteryProtectionPolicy -> Battery normal (SOC=%u%%)", soc);
}

#endif // PLATFORM_ARDUINO