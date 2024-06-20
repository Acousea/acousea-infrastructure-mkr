#ifndef IROUTINE_H
#define IROUTINE_H

#include <Arduino.h>
#include "../Packet/Packet.h"

class IRoutine {
public:
    virtual Packet execute(const Packet& packet) = 0;
};

#endif // IROUTINE_H
