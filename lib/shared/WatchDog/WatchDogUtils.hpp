#ifndef ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOG_UTILS_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOG_UTILS_HPP

#include <cstdint>

namespace WatchdogUtils
{
    constexpr unsigned int DEFAULT_WATCHDOG_TIMEOUT_MS = 15000; // 15 seconds

    void enable(int timeoutMs);
    void reset();
    void disable();
    void sleepFor(uint32_t ms);
    void logResetCause();
    [[nodiscard]] int getTimeout();
}


#endif //ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOG_UTILS_HPP