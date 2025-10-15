#ifndef ADAFRUITLC_MANAGER_H
#define ADAFRUITLC_MANAGER_H
#ifdef ARDUINO

#include "IBatteryController.h"


class AdafruitLCBatteryController final : public IBatteryController {
public:
    bool init() override;

    uint8_t voltageSOC_rounded() override;

    acousea_BatteryStatus  status() override;

    float voltage();

    float temperature();

};

#endif // ARDUINO

#endif // AdafruitLC_MANAGER_H