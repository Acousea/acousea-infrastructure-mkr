#ifndef IPORT_H
#define IPORT_H

#include <Arduino.h>
#include "Packet.h"

// Definición de la interfaz de comunicador
class IPort {
public:
    virtual void init() = 0;
    virtual void send(const Packet& packet) = 0;
    virtual bool available() = 0;
    virtual Packet read() = 0;
};

#endif // IPORT_H
