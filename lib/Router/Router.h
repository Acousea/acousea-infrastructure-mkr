#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <deque>
#include <map>
#include <Ports/IPort.h>
#include <Packet.h>
#include <Result/Result.h>

//// Extra dependencies (might not be necessary)




/**
 * @brief Router class that relays packets between ports and processes them if necessary.
 */
class Router {
private:
    std::vector<IPort *> relayedPorts;
    std::map<IPort::PortType, std::deque<Packet>> receivedPackets = {};

    // Clase interna para manejar el envío con una dirección
    class RouterSender {
    private:
        Address localAddress;
        Router *router;

        [[nodiscard]] Packet configurePacketRouting(const Packet &packet) const {
            Packet mutablePacket = packet;
            mutablePacket.setRoutingChunk(RoutingChunk(
                    localAddress,
                    packet.getRoutingChunk().getSender())
            );
            return mutablePacket;
        }

    public:
        explicit RouterSender(Address address, Router *router)
                : localAddress(address), router(router) {}


        void sendSBD(const Packet &packet) {
            Packet mutablePacket = configurePacketRouting(packet);
            router->sendSBD(mutablePacket);
        }


        void sendLoRa(const Packet &packet) {
            Packet mutablePacket = configurePacketRouting(packet);
            router->sendLoRa(mutablePacket);
        }

        void sendSerial(const Packet &packet) {
            Packet mutablePacket = configurePacketRouting(packet);
            router->sendSerial(mutablePacket);
        }
    };

public:
    Router(const std::vector<IPort *> &relayedPorts);

    void addRelayedPort(IPort *port);

    Router::RouterSender sendFrom(Address senderAddress);

    /**
        * @brief Reads packets from the ports and returns them grouped by port type.
        * @param localAddress The local address of the router.
        * @return A map where keys are port types and values are lists of packets received from those ports.
        */
    std::map<IPort::PortType, std::deque<Packet>> readPorts(const Address &localAddress);

private:
    void sendSBD(const Packet &packet);

    void sendLoRa(const Packet &packet);

    void sendSerial(const Packet &packet);



};

#endif // COMMUNICATOR_RELAY_H
