#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <Arduino.h>
#include <map>
#include "IProcessor.h"
#include "../Display/IDisplay.h"
#include "../Routines/IRoutine.h"
#include "../Packet/ErrorPacket.h"

class PacketProcessor : public IProcessor {
private:
    IDisplay* display;
    std::map<uint8_t, IRoutine*> routines;

public:
    PacketProcessor(IDisplay* display, const std::map<uint8_t, IRoutine*>& routines)
        : display(display), routines(routines) {}

    Packet process(const Packet& packet) override {
        display->print("Packet Processor: ");
        display->print(packet.getFullPacketVector());
        uint8_t opCode = packet.getOpCode();        
        if (routines.find(opCode) != routines.end()) {
            return routines[opCode]->execute(packet);
        } else {
            display->print("Exception: Invalid OpCode: no routine found for OpCode");
            packet.swapSenderReceiverAddresses();
            return ErrorPacket(packet.getAddresses(), ErrorPacket::ErrorCode::INVALID_OPCODE);
        }
    }
};

#endif