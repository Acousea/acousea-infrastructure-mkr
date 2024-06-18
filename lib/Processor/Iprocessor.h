#ifndef IPROCESSOR_H
#define IPROCESSOR_H

#include <Arduino.h>

class IProcessor {
public:
    virtual void processMessage(uint8_t* data, size_t length) = 0;
};

#endif