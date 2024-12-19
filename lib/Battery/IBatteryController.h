#ifndef IBATTERY_H
#define IBATTERY_H

#include <cstdint>

class IBatteryController
{
public:
    virtual bool init() = 0;
    virtual uint8_t percentage() = 0;
    virtual uint8_t status() = 0;

};

#endif // IBATTERY_H
