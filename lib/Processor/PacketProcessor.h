#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <Arduino.h>
#include <map>
#include "IProcessor.h"
#include "../Display/IDisplay.h"
#include "../Routines/IRoutine.h"

class PacketProcessor : public IProcessor {
private:
    IDisplay* display;
    std::map<uint8_t, IRoutine*> routines;

public:
    PacketProcessor(IDisplay* display, const std::map<uint8_t, IRoutine*>& routines)
        : display(display), routines(routines) {}

    Packet processPacket(const Packet& packet) override {
        display->print("Message Processor: ");
        display->print(packet.getFullPacketVector());

        uint8_t opCode = packet.getOpCode();
        if (routines.find(opCode) != routines.end()) {
            return routines[opCode]->execute(packet);
        } else {
            throw std::invalid_argument("Invalid OpCode: no routine found for OpCode");            
        }
    }
};

#endif