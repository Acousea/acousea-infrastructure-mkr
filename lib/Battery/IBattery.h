#ifndef IBATTERY_H
#define IBATTERY_H

#include <Arduino.h>


class IBattery
{
public:
    virtual bool init() = 0;
    virtual uint8_t percentage() = 0;

};

#endif // IBATTERY_H
