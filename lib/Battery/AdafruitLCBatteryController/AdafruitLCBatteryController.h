#ifndef ADAFRUITLC_MANAGER_H
#define ADAFRUITLC_MANAGER_H


#include "Adafruit_LC709203F.h"
#include "IBatteryController.h"

class AdafruitLCBatteryController : public IBatteryController {
public:
    bool init() override;

    uint8_t percentage() override;

    uint8_t status() override;

    float voltage();

    float temperature();

private:
    Adafruit_LC709203F adafruitLC; // Battery controller
};

#endif // AdafruitLC_MANAGER_H