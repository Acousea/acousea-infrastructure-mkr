#ifndef ACOUSEA_WATCHDOG_MANAGER_H
#define ACOUSEA_WATCHDOG_MANAGER_H

#ifdef ARDUINO

#include <Adafruit_SleepyDog.h>
#include "ClassName.h"
#include "Logger/Logger.h"


class WatchDogMonitor {
    CLASS_NAME(SystemMonitor)

public:
    WatchDogMonitor() = default;

    static void init(int timeoutMs);

    static void reset();

private:
    static void logResetCause();
};


#endif // ARDUINO

#endif //ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOGMANAGER_H
