#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <Arduino.h>
#include <vector>
#include "../Port/IPort.h"
#include "../Port/SerialPort.h"
#include "../Processor/PacketProcessor.h"
#include "../RoutingTable/RoutingTable.h"

/**
 * @brief Relay class that acts as a bridge between two ports and a processor
 * 
 * This class is responsible for relaying packets between two ports and a processor.
 * If the packet is for this device, it will be processed by the processor. 
 * Otherwise, it will be relayed to other port, depending on the recipient address.
 * 
 *  BACKEND <-> LOCALIZER <-> DRIFTER <-> PI3   
 * 
 *  PrimaryPort --- [[MKR 1310]] --- SecondaryPort 
 *  
 *  FORWARDING RULES: (Drifter) -> Scheme:  LoraPort --- [[MKR 1310]] --- SerialPort
 *  - Messages to the drifter/localizer from the backend come through the primary port, which can be 
 *    either LoRa or Serial port.
 *  - Messages to the drifter from the PI3 come through the secondary port, which is the Serial port.
 * 
 *  FORWARDING RULES: (Localizer) -> Scheme:  SerialPort --- [[MKR 1310]] --- LoraPort
 *  - Messages to the localizer from the drifter/PI3 come through the secondary port, whicih can be 
 *    either LoRa or Serial port.
 *  - Messages from the backend to the localizer come through the primary port, which is Serial port.  * 
 * 
 *  In this scenario, the primary and secondary ports can be inverted, depending on the device running the code.
 */
class Router {
private:
    Packet::Address localAddress; // Dirección local del dispositivo
    RoutingTable* routingTable; // La tabla de ruteo
    PacketProcessor* processor;             
    IDisplay* display;
    std::vector<IPort*> relayedPorts;

public:
    Router(Packet::Address localAddress, RoutingTable* routingTable, PacketProcessor* processor, IDisplay* display) 
        : localAddress(localAddress), routingTable(routingTable), processor(processor), display(display) {}

    // Constructor que recibe un vector de comunicadores
    Router(Packet::Address localAddress, RoutingTable* routingTable, PacketProcessor* processor, IDisplay* display, std::vector<IPort*> ports) 
        : localAddress(localAddress), routingTable(routingTable), processor(processor), display(display), relayedPorts(ports) {}

    void addRelayedPort(IPort* port) {
        relayedPorts.push_back(port);
    }

    void send(const Packet& packet) {
        route(packet);
    }

    void setLocalAddress(Packet::Address address) {
        localAddress = address;
    }
    
    void relayPorts() {
        for (auto port = relayedPorts.begin(); port != relayedPorts.end(); ++port) {            
            if ((*port)->available()) {
                display->print("Router::operate() -> Port available");
                route((*port)->read()); // Route the packet                
            }
        }
    }

    /**
     * @brief Route the packet to the correct port
     * It either processes the packet or relays it to the correct port
     * A processed packet may be re-routed. 
     * Null packets are not processed nor relayed, since they neither have no routine associated nor a valid addresses byte.
     */
    void route(const Packet& packet) {
        // Print the packet HEX
        display->print("Router::route() -> Packet: ");
        packet.print();
        
        uint8_t address = packet.getAddresses();
        if ((GET_RECEIVER(address) == localAddress) && processor != nullptr) {
            display->print("Router::route-if() -> Processing packet...");
            SerialUSB.println(address, HEX);
            route(processor->process(packet)); // Process the packet and re-route it
        } else {
            // Print address as HEX
            display->print("Router::route-else() -> Relaying packet to address: ");
            SerialUSB.println(address, HEX);            
            IPort* port = routingTable->getRoute(address);
            if (port != nullptr) {
                port->send(packet);
            } else { // Discard the pacekt                
                display->print("Exception: Invalid Address: no route found for address");
            }            
        }
        
    }
};

#endif // COMMUNICATOR_BRIDGE_H