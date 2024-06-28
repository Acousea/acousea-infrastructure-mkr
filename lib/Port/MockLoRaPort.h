#ifndef MOCK_LORA_PORT_H
#define MOCK_LORA_PORT_H

#include <Arduino.h>
#include "IPort.h"
#include "Packet.h"

class MockLoRaPort : public IPort {
public:

    void init() override {
        // Print through serial monitor
        SerialUSB.println("MockLoRaPort: Initializing LoRa port");
    }
    
    void send(const Packet& packet) override {
        // Print packet through serial monitor for debugging
        SerialUSB.print("MockLoRaPort: Sending packet: ");
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


#endif // MOCK_LORA_PORT_H
