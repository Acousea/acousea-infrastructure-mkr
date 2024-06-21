#ifndef MOCK_LORA_COMMUNICATOR_H
#define MOCK_LORA_COMMUNICATOR_H

#include <Arduino.h>
#include "ICommunicator.h"
#include "Packet.h"

class MockLoraCommunicator : public ICommunicator {
public:
    void send(const Packet& packet) override {
        // Print packet through serial monitor for debugging
        SerialUSB.print("MockLoraCommunicator: Sending packet: ");
        for (size_t i = 0; i < packet.getFullPacketLength(); i++) {
            SerialUSB.print(packet.getFullPacket()[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();

    }

    bool available() override {
        return false;        
    }

    Packet read() override {
        uint8_t mock_data[4] = {0x20, 0x01, 0x02, 0x03};
        return Packet(mock_data, 0); 
    }

private:
   
};


#endif // MOCK_LORA_COMMUNICATOR_H
