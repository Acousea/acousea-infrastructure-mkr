#ifndef LORA_COMMUNICATOR_H
#define LORA_COMMUNICATOR_H

#include <Arduino.h>
#include <vector>
#include "ICommunicator.h"

class LoraCommunicator : public ICommunicator {
public:
    void send(const uint8_t* data, size_t length) override {
        Serial1.write(data, length);
    }

    bool available() override {
        // return Serial1.available();
        return false;
    }

    std::vector<uint8_t> read() override {
        std::vector<uint8_t> data;
        while (Serial1.available()) {
            data.push_back(Serial1.read());
        }
        return data;
    }
};

#endif