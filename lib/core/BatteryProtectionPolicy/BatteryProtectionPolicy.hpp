#ifndef ACOUSEA_INFRASTRUCTURE_MKR_BATTERY_PROTECTION_POLICY_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_BATTERY_PROTECTION_POLICY_HPP

#ifdef PLATFORM_ARDUINO

#include "IBatteryController.h"
#include "PiController/PiController.hpp"

class BatteryProtectionPolicy
{
public:
    BatteryProtectionPolicy(IBatteryController& battery, PiController& pi)
        : batteryCtrl(battery), piCtrl(pi) {}

    void enforce() const;

private:
    IBatteryController& batteryCtrl;
    PiController& piCtrl;
};


#endif // PLATFORM_ARDUINO

#endif //ACOUSEA_INFRASTRUCTURE_MKR_BATTERY_PROTECTION_POLICY_HPP