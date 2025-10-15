#ifndef ACOUSEA_WATCHDOG_MANAGER_H
#define ACOUSEA_WATCHDOG_MANAGER_H

#ifdef ARDUINO

#include <Adafruit_SleepyDog.h>
#include "ClassName.h"
#include "Logger/Logger.h"
#include "IBatteryController.h"
#include "RockPiPowerController/RockPiPowerController.hpp"
#include "time/getMillis.hpp"


class SystemMonitor
{
    CLASS_NAME(SystemMonitor)

public:
    SystemMonitor(IBatteryController* batteryCtrl, RockPiPowerController* rockpiCtrl)
        : batteryController(batteryCtrl), rockpiController(rockpiCtrl)
    {
    }

    void init(int timeoutMs);

    static void reset();

    void protectBattery() const;

private:
    static void logResetCause();

    void sleepFor(uint32_t ms) const;
    void manageRockPiAction(unsigned long cooldownMs, void (RockPiPowerController::*action)() const) const;

private:
    int watchDogTimeoutMs = 5000; // Default 5 seconds
    IBatteryController* batteryController;
    RockPiPowerController* rockpiController;
};


#endif // ARDUINO

#endif //ACOUSEA_INFRASTRUCTURE_MKR_WATCHDOGMANAGER_H
