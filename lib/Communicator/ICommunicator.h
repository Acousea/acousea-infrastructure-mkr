#ifndef ICOMMUNICATOR_H
#define ICOMMUNICATOR_H

#include <Arduino.h>
#include <vector>

// Definición de la interfaz de comunicador
class ICommunicator {
public:
    virtual void send(const uint8_t* data, size_t length) = 0;
    virtual bool available() = 0;
    virtual std::vector<uint8_t> read() = 0;
};

#endif