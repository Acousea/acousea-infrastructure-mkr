#ifndef IBATTERY_H
#define IBATTERY_H

#include <cstdint>
#include "bindings/nodeDevice.pb.h"

class IBatteryController
{
protected:
    ~IBatteryController() = default;

public:
    virtual bool init() = 0;
    virtual uint8_t voltageSOC_rounded() = 0;
    virtual acousea_BatteryStatus  status() = 0;

};

#endif // IBATTERY_H
