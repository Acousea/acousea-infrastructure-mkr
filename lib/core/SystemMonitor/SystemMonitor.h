#ifndef ACOUSEA_WATCHDOG_MANAGER_H
#define ACOUSEA_WATCHDOG_MANAGER_H

#ifdef ARDUINO

#include <Adafruit_SleepyDog.h>

#include "ClassName.h"
#include "IBatteryController.h"
#include "RockPiPowerController/RockPiPowerController.hpp"
#include "Logger/Logger.h"


class SystemMonitor {
    CLASS_NAME(SystemMonitor)

public:
    SystemMonitor(IBatteryController *batteryController, RockPiPowerController *rockpiPowerController)
        : batteryController(batteryController), rockpiPowerController(rockpiPowerController) {

    }

    static void init(uint32_t timeoutMs = 8000);

    static void reset();

private:
    static void logResetCause();
    IBatteryController* batteryController;
    RockPiPowerController* rockpiPowerController;
};


#endif // ARDUINO

#endif //ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOGMANAGER_H
