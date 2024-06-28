#ifndef MOCK_IRIDIUM_PORT_H
#define MOCK_IRIDIUM_PORT_H

#include <Arduino.h>
#include "IPort.h"

class MockIridiumPort : public IPort {
public:
    void init() override {
        SerialUSB.println("MockIridiumPort: Initializing Iridium port");
    }

    void send(const Packet& packet) override {                
        SerialUSB.print("MockIridiumPort: Sending packet: ");
        packet.print();
    }

    bool available() override {        
        return false;
    }

    Packet read() override {        
        uint8_t mock_data[4] = {0x20, 0x01, 0x02, 0x03};
        return Packet(mock_data, 0);
    }
};

#endif // MOCK_IRIDIUM_PORT_H