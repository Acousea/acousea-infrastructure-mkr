#ifndef ACOUSEA_WATCHDOG_MANAGER_H
#define ACOUSEA_WATCHDOG_MANAGER_H

#ifdef ARDUINO

#include <Adafruit_SleepyDog.h>
#include "ClassName.h"
#include "Logger/Logger.h"
#include "IBatteryController.h"
#include "RockPiPowerController/RockPiPowerController.hpp"


class SystemMonitor {
    CLASS_NAME(SystemMonitor)

public:
    SystemMonitor(IBatteryController *batteryCtrl, RockPiPowerController *rockpiCtrl)
        : batteryController(batteryCtrl), rockpiController(rockpiCtrl) {
    }

    void sync() const;

    static void init(int timeoutMs);


private:
    static void logResetCause();

    void checkBatteryProtection() const;

private:
    IBatteryController *batteryController;
    RockPiPowerController *rockpiController;
};


#endif // ARDUINO

#endif //ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOGMANAGER_H
