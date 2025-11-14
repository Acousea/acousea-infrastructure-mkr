#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <vector>
#include <Ports/IPort.h>
#include <Result.h>

#include "ClassName.h"
#include "bindings/nodeDevice.pb.h"
#include "PacketQueue/PacketQueue.hpp"


/**
 * @brief Router class that relays packets between ports and processes them if necessary.
 */
class Router
{
    CLASS_NAME(Router)

public:
    static constexpr uint8_t originAddress = 0;
    static constexpr uint8_t broadcastAddress = 255;

    Router(
        const std::vector<IPort*>& ports,
        const std::vector<IPort::PortType>& relayedPortTypes,
        PacketQueue& packetQueue
    );

    void addPort(IPort* port);

    void addRelayedPortType(IPort::PortType portType);

    void relayPacket(const acousea_CommunicationPacket& inPacket) const;

    class RouterSender;

    [[nodiscard]] Router::RouterSender from(uint8_t sender) const;

    [[nodiscard]] Router::RouterSender broadcast();
    /**
        * @brief Reads packets from the ports and returns the next available packet.
        * @param localAddress The local address of the router.
        * @return An optional pair containing the port type and the communication packet, or std::nullopt if no packet is available.
        */

    [[nodiscard]] bool syncAllPorts() const;

    [[nodiscard]] std::optional<std::pair<IPort::PortType, acousea_CommunicationPacket*>> peekNextPacket(
        uint8_t localAddress) const;

    [[nodiscard]] bool skipToNextPacket(IPort::PortType portType) const;

    // ======================================================
    // Builder interno para API fluida
    // ======================================================
    class RouterSender
    {
    public:
        // friend class Router; // Permite a Router acceder a constructores privados
        friend Router::RouterSender Router::from(uint8_t) const;
        friend Router::RouterSender Router::broadcast();

        explicit RouterSender(const Router* router, const uint8_t sender_address);

    private:
        explicit RouterSender(const Router* router);

    public:
        void setSender(const uint8_t sender_address)
        {
            senderAddress = sender_address;
        }

        void setReceiver(const uint8_t receiver_address)
        {
            receiverAddress = receiver_address;
        }

        // Etapa 1: seleccionar puerto
        RouterSender& through(IPort::PortType type);

        RouterSender& to(uint8_t receiver);

        // Etapa 2: enviar paquete
        // bool send(const acousea_CommunicationPacket* pkt) const;
        [[nodiscard]] bool send(acousea_CommunicationPacket& pkt) const;

    private:
        const Router* router;
        uint8_t senderAddress{broadcastAddress};
        uint8_t receiverAddress{broadcastAddress};
        IPort::PortType selectedPort{IPort::PortType::SerialPort};
    };

private:
#ifdef UNIT_TESTING
    friend class TestableRouter;
#endif

    std::vector<IPort*> ports_{};
    std::vector<IPort::PortType> relayedPortTypes_{};
    PacketQueue& packetQueue_;

    [[nodiscard]] bool sendToPort(IPort::PortType port, const acousea_CommunicationPacket& packet) const;
};

#endif // COMMUNICATOR_RELAY_H
