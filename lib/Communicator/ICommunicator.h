#ifndef ICOMMUNICATOR_H
#define ICOMMUNICATOR_H

#include <Arduino.h>
#include "Packet.h"

// Definición de la interfaz de comunicador
class ICommunicator {
public:
    virtual void send(const Packet& packet) = 0;
    virtual bool available() = 0;
    virtual Packet read() = 0;
};

#endif // ICOMMUNICATOR_H
