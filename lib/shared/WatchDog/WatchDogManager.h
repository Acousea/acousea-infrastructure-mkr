#ifndef ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOGMANAGER_H
#define ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOGMANAGER_H

#ifdef ARDUINO

#include <Adafruit_SleepyDog.h>
#include "Logger/Logger.h"

class WatchdogManager {
public:
    static void init(uint32_t timeoutMs = 8000);

    static void reset();

private:
    static void logResetCause();
};


#endif // ARDUINO

#endif //ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOGMANAGER_H
