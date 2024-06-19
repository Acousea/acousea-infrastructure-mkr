#ifndef IRIDIUM_COMMUNICATOR_H
#define IRIDIUM_COMMUNICATOR_H

#include <Arduino.h>
#include <vector>
#include "ICommunicator.h"

class IridiumCommunicator : public ICommunicator {
public:
    void send(const Packet& packet) override {
        // TODO: Implement Iridium send
        // Print through serial monitor
        Serial.print("IRIDIUM_COMMUNICATOR: Sending packet: ");
        for (size_t i = 0; i < packet.getFullPacketLength(); i++) {
            Serial.print(packet.getFullPacket()[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    bool available() override {
        // TODO: Implement Iridium available
        return false;
    }

    Packet read() override {
        // TODO: Implement Iridium read
        uint8_t mock_data[4] = {0x20, 0x01, 0x02, 0x03};
        return Packet(mock_data, 0);
    }
};

#endif