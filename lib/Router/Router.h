#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H


#include <utility>
#include <vector>
#include "Processor/PacketProcessor.h"
#include "Ports/IPort.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"


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
    PacketProcessor *processor;
    IDisplay *display;
    std::vector<IPort *> relayedPorts;
    NodeConfigurationRepository nodeConfigurationRepository;

    // Clase interna para manejar el envío con una dirección
    class RouterSender {
    private:
        Address localAddress;
        Router *router;

    public:
        explicit RouterSender(Address address, Router *router)
                : localAddress(address), router(router) {}

        void sendSBD(const Packet &packet) {
            router->sendSBD(packet);
        }

        void sendLoRa(const Packet &packet) {
            router->sendLoRa(packet);
        }

        void sendSerial(const Packet &packet) {
            router->sendSerial(packet);
        }
    };

public:
    Router(PacketProcessor *processor, IDisplay *display, const std::vector<IPort *> &relayedPorts,
           const NodeConfigurationRepository &nodeConfigurationRepository);

    void addRelayedPort(IPort *port);

    RouterSender sender();

    void readPorts();

    void processPacket(IPort* port, Packet &requestPacket);

private:

    void sendSBD(const Packet &packet);

    void sendLoRa(const Packet &packet);

    void sendSerial(const Packet &packet);
};

#endif // COMMUNICATOR_BRIDGE_H