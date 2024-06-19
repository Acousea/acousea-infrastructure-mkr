#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <Arduino.h>
#include <vector>
#include "../Communicator/ICommunicator.h"
#include "../Processor/IProcessor.h"


// Clase principal que actúa como puente
class CommunicatorRelay {
private:
    ICommunicator* primaryCommunicator; // El comunicador principal
    ICommunicator* secondaryCommunicator; // El comunicador secundario
    IProcessor* processor;        

public:
    CommunicatorRelay(ICommunicator* primaryCommunicator, ICommunicator* secondaryCommunicator, IProcessor* processor) 
        : primaryCommunicator(primaryCommunicator), secondaryCommunicator(secondaryCommunicator), processor(processor) {}

    void relayFromPrimary() {
        if (primaryCommunicator->available()) {
            auto packet = primaryCommunicator->read();

            uint8_t address = packet.getAddress();
            if (address == Packet::Address::PI3 || address == Packet::Address::LOCALIZER) {
                secondaryCommunicator->send(packet);
            } else {
                processor->processPacket(packet);
            }
        }
    }

    void relayFromSecondary() {
        while (secondaryCommunicator->available()) {
            auto packet = secondaryCommunicator->read();

            uint8_t address = packet.getAddress();
            if (address == Packet::Address::PI3 || address == Packet::Address::LOCALIZER) {
                primaryCommunicator->send(packet);
            } else {
                processor->processPacket(packet);
            }
        }
    }
};

#endif // COMMUNICATOR_BRIDGE_H