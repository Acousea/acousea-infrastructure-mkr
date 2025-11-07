#ifndef ADAFRUITLC_MANAGER_H
#define ADAFRUITLC_MANAGER_H
#ifdef PLATFORM_ARDUINO

#include "ClassName.h"
#include "IBatteryController.h"


class AdafruitLCBatteryController final : public IBatteryController
{
    CLASS_NAME(AdafruitLCBatteryController)
    bool errorState = true;

public:
    bool init() override;

    bool sync() override;

    uint8_t voltageSOC_rounded() override;

    acousea_BatteryStatus status() override;

    float voltage();

    float temperature();
};

#endif // PLATFORM_ARDUINO

#endif // AdafruitLC_MANAGER_H
