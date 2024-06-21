#ifndef IPROCESSOR_H
#define IPROCESSOR_H

#include <Arduino.h>

class IProcessor {
public:
    virtual Packet process(const Packet& packet) = 0;
};

#endif