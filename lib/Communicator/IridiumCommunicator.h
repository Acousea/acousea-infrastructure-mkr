#ifndef IRIDIUM_COMMUNICATOR_H
#define IRIDIUM_COMMUNICATOR_H

#include <Arduino.h>
#include <vector>
#include "ICommunicator.h"

class IridiumCommunicator : public ICommunicator {
public:
    void send(const uint8_t* data, size_t length) override {
        Serial2.write(data, length);
    }

    bool available() override {
        return Serial2.available();
    }

    std::vector<uint8_t> read() override {
        std::vector<uint8_t> data;
        while (Serial2.available()) {
            data.push_back(Serial2.read());
        }
        return data;
    }
};

#endif