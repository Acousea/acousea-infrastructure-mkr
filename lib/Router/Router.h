#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <deque>
#include <map>
#include <Ports/IPort.h>
#include <Result.h>
#include "bindings/nodeDevice.pb.h"


/**
 * @brief Router class that relays packets between ports and processes them if necessary.
 */
class Router
{
public:
    static constexpr uint8_t originAddress = 0;
    static constexpr uint8_t broadcastAddress = 255;

private:
    std::vector<IPort*> relayedPorts;


    // Clase interna para manejar el envío con una dirección
    class RouterSender
    {
    private:
        uint8_t localAddress;
        Router* router;

    private:
        [[nodiscard]] acousea_CommunicationPacket configurePacketRouting(acousea_CommunicationPacket& inPacket) const;

    public:
        explicit RouterSender(uint8_t address, Router* router);

        void sendSBD(acousea_CommunicationPacket& packet) const;
        void sendLoRa(acousea_CommunicationPacket& packet) const;
        void sendSerial(acousea_CommunicationPacket& packet) const;
    };

public:
    explicit Router(const std::vector<IPort*>& relayedPorts);

    void addRelayedPort(IPort* port);

    Router::RouterSender sendFrom(uint8_t senderAddress);

    /**
        * @brief Reads packets from the ports and returns them grouped by port type.
        * @param localAddress The local address of the router.
        * @return A map where keys are port types and values are lists of packets received from those ports.
        */
    std::map<IPort::PortType, std::deque<acousea_CommunicationPacket>> readPorts(const uint8_t& localAddress);

private:
    // ---------------------- Packet encoding/decoding ----------------------
    static Result<acousea_CommunicationPacket> decodePacket(const std::vector<uint8_t>& raw);
    static Result<std::vector<uint8_t>> encodePacket(const acousea_CommunicationPacket& pkt);

    // ---------------------- Sending to specific ports ----------------------
    void sendSBD(const acousea_CommunicationPacket& packet) const;

    void sendLoRa(const acousea_CommunicationPacket& packet) const;

    void sendSerial(const acousea_CommunicationPacket& packet) const;
};

#endif // COMMUNICATOR_RELAY_H
