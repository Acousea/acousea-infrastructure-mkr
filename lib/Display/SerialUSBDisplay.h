#ifndef SERIAL_USB_DISPLAY_H
#define SERIAL_USB_DISPLAY_H

#include <Arduino.h>
#include "IDisplay.h"

class SerialUSBDisplay : public IDisplay {
public:
    void init(int baudRate = 9600) {
        SerialUSB.begin(baudRate);
        while (!SerialUSB) {
            delay(10);
        }
        SerialUSB.println("SerialUSBDisplay initialized");
    }

    void print(const uint8_t* data, size_t length) override {
        for (size_t i = 0; i < length; i++) {
            SerialUSB.print(data[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();
    }

    virtual void print(const std::vector<uint8_t>& data) {
        for (size_t i = 0; i < data.size(); i++) {
            SerialUSB.print(data[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();
    }

    void print(const String& message) override {
        SerialUSB.println(message);
    }

   void print(const char *message) override {
        SerialUSB.println(message);
    }

    void clear() override {
        SerialUSB.println("SerialUSBDisplay cleared");
    }
        
};

#endif
