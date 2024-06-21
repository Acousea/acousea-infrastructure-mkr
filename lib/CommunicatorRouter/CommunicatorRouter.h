#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <Arduino.h>
#include <vector>
#include "../Communicator/ICommunicator.h"
#include "../Communicator/SerialCommunicator.h"
#include "../Processor/IProcessor.h"
#include "../RoutingTable/RoutingTable.h"

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
    Packet::Address localAddress; // Dirección local del dispositivo
    RoutingTable* routingTable; // La tabla de ruteo
    IProcessor* processor;             
    std::vector<ICommunicator*> relayedCommunicators;

public:
   CommunicatorRouter(Packet::Address localAddress, RoutingTable* routingTable, IProcessor* processor) 
        : localAddress(localAddress), routingTable(routingTable), processor(processor) {}

    // Constructor que recibe un vector de comunicadores
    CommunicatorRouter(Packet::Address localAddress, RoutingTable* routingTable, IProcessor* processor, std::vector<ICommunicator*> communicators) 
        : localAddress(localAddress), routingTable(routingTable), processor(processor), relayedCommunicators(communicators) {}

    void addRelayedCommunicator(ICommunicator* communicator) {
        relayedCommunicators.push_back(communicator);
    }

    void operate() {
        relayCommunicators();        
        delay(100); // Pequeño retraso para evitar saturar el puerto serial               
    }    

    void setLocalAddress(Packet::Address address) {
        localAddress = address;
    }
    
    void relayCommunicators() {
        for (auto communicator = relayedCommunicators.begin(); communicator != relayedCommunicators.end(); ++communicator) {
            if ((*communicator)->available()) {
                route((*communicator)->read()); // Route the packet
                delay(100);                     // Small delay to avoid saturating the serial port
            }
        }
    }

    void route(const Packet& packet) {
        uint8_t address = packet.getRecipientAddress() | packet.getPacketType();
        if (address == localAddress) {
            route(processor->process(packet)); // Process the packet and re-route it
        } else {
            ICommunicator* communicator = routingTable->getRoute(address);
            if (communicator != nullptr) {
                communicator->send(packet);
            } else { // Discard the pacekt                
                SerialUSB.println("Exception: Invalid Address: no route found for address");
            }            
        }
        
    }
};

#endif // COMMUNICATOR_BRIDGE_H