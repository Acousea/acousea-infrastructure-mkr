#ifndef LORA_COMMUNICATOR_H
#define LORA_COMMUNICATOR_H

#include <Arduino.h>
#include <vector>
#include "ICommunicator.h"

class LoraCommunicator : public ICommunicator {
public:
    void send(const Packet& packet) override {
        // TODO: Implement LoRa send
        // Print through serial monitor
        Serial.print("LORA_COMMUNICATOR: Sending packet: ");
        for (size_t i = 0; i < packet.getFullPacketLength(); i++) {
            Serial.print(packet.getFullPacket()[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

    }

    bool available() override {
        // TODO: Implement LoRa available
        return false;
    }

    Packet read() override {
        // TODO: Implement LoRa read
        // Create mock buffer of length 4
        uint8_t mock_data[4] = {0x20, 0x01, 0x02, 0x03};
        return Packet(mock_data, 0);
    }
};

#endif