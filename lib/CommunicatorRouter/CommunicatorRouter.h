#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <Arduino.h>
#include <vector>
#include "../Communicator/ICommunicator.h"
#include "../Communicator/SerialCommunicator.h"
#include "../Processor/IProcessor.h"

/**
 * @brief Relay class that acts as a bridge between two communicators and a processor
 * 
 * This class is responsible for relaying packets between two communicators and a processor.
 * If the packet is for this device, it will be processed by the processor. 
 * Otherwise, it will be relayed to other communicator, depending on the recipient address.
 * 
 *  BACKEND <-> LOCALIZER <-> DRIFTER <-> PI3   
 * 
 *  PrimaryCommunicator --- [[MKR 1310]] --- SecondaryCommunicator 
 *  
 *  FORWARDING RULES: (Drifter) -> Scheme:  LoraCommunicator --- [[MKR 1310]] --- SerialCommunicator
 *  - Messages to the drifter/localizer from the backend come through the primary communicator, which can be 
 *    either LoRa or Serial communicator.
 *  - Messages to the drifter from the PI3 come through the secondary communicator, which is the Serial communicator.
 * 
 *  FORWARDING RULES: (Localizer) -> Scheme:  SerialCommunicator --- [[MKR 1310]] --- LoraCommunicator
 *  - Messages to the localizer from the drifter/PI3 come through the secondary communicator, whicih can be 
 *    either LoRa or Serial communicator.
 *  - Messages from the backend to the localizer come through the primary communicator, which is Serial communicator.  * 
 * 
 *  In this scenario, the primary and secondary communicators can be inverted, depending on the device running the code.
 */
class CommunicatorRouter {
private:
    ICommunicator* primaryCommunicator; // El comunicador principal
    ICommunicator* secondaryCommunicator; // El comunicador secundario
    IProcessor* processor;            
    Packet::Address peerAddress; 
    
    CommunicatorRouter(ICommunicator* primaryCommunicator, ICommunicator* secondaryCommunicator, IProcessor* processor, Packet::Address peerAddress) 
        : primaryCommunicator(primaryCommunicator), secondaryCommunicator(secondaryCommunicator), processor(processor), peerAddress(peerAddress) {}

public:
    static CommunicatorRouter* createLocalizerRouter(SerialCommunicator* primaryCommunicator, ICommunicator* secondaryCommunicator, IProcessor* processor) {
        return new CommunicatorRouter(primaryCommunicator, secondaryCommunicator, processor, Packet::Address::DRIFTER);
    }

    static CommunicatorRouter* createDrifterRouter(ICommunicator* primaryCommunicator, SerialCommunicator* secondaryCommunicator, IProcessor* processor) {
        return new CommunicatorRouter(primaryCommunicator, secondaryCommunicator, processor, Packet::Address::LOCALIZER);
    }

    void doBidirectionalRouting() {
        routeFromPrimaryToSecondary();        
        delay(100); // Pequeño retraso para evitar saturar el puerto serial

        routeFromSecondaryToPrimary();
        delay(100); // Pequeño retraso para evitar saturar el puerto serial
    }
    
    void routeFromPrimaryToSecondary() { // 
        if (!primaryCommunicator->available()) {
            return;
        }        
        auto packet = primaryCommunicator->read();
        route(packet);        
    }

    void routeFromSecondaryToPrimary() {
        if (!secondaryCommunicator->available()) {
            return;
        }
        auto packet = secondaryCommunicator->read();
        route(packet);           
    }

    void route(const Packet& packet) {
        uint8_t address = packet.getRecipientAddress();
        if (address == Packet::Address::BACKEND) {
            primaryCommunicator->send(packet);
        } else if (address == this->peerAddress) {
            if (this->peerAddress == Packet::Address::LOCALIZER) {
                primaryCommunicator->send(packet); // Localizer
            } else { 
                secondaryCommunicator->send(packet); // Drifter
            }
        } else if (address == Packet::Address::PI3) {
            secondaryCommunicator->send(packet);
        } else {
            route(processor->processPacket(packet));           
        }
    }
};

#endif // COMMUNICATOR_BRIDGE_H