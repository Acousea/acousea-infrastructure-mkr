#ifndef IDISPLAY_H
#define IDISPLAY_H

#include <Arduino.h>

class IDisplay
{
public:
    virtual void print(const uint8_t *data, size_t length) = 0;
    virtual void print(const std::vector<uint8_t> &data) = 0; // Cambio de std::vector<uint8_t> a String (linea 33
    virtual void print(const String &message) = 0;
    virtual void print(const char *message) = 0;    
    virtual void clear() = 0;
};

#endif