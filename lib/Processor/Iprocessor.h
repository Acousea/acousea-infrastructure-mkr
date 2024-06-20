#ifndef IPROCESSOR_H
#define IPROCESSOR_H

#include <Arduino.h>

class IProcessor {
public:
    virtual Packet processPacket(const Packet& packet) = 0;
};

#endif