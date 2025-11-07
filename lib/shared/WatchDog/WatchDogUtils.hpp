#ifndef ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOG_UTILS_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOG_UTILS_HPP

#include <cstdint>

namespace WatchdogUtils
{
    void enable(int timeoutMs);
    void reset();
    void disable();
    void sleepFor(uint32_t ms);
    void logResetCause();
}


#endif //ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOG_UTILS_HPP