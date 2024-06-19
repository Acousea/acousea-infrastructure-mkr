#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <Arduino.h>
#include "IProcessor.h"
#include "../Display/IDisplay.h"

class MessageProcessor : public IProcessor {
private:
    IDisplay* display;

public:
    MessageProcessor(IDisplay* display) : display(display) {}

    void processPacket(const Packet& packet) override {
        display->print("Message Processor: ");
        display->print(packet.getFullPacketVector());
    }
};

#endif