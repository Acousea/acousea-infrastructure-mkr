#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <vector>
#include <Ports/IPort.h>
#include <Result.h>

#include "ClassName.h"
#include "bindings/nodeDevice.pb.h"


/**
 * @brief Router class that relays packets between ports and processes them if necessary.
 */
class Router
{
    CLASS_NAME(Router)

public:
    static constexpr uint8_t originAddress = 0;
    static constexpr uint8_t broadcastAddress = 255;

    explicit Router(const std::vector<IPort*>& relayedPorts);

    void addRelayedPort(IPort* port);

    class RouterSender;

    [[nodiscard]] Router::RouterSender from(uint8_t sender) const;

    [[nodiscard]] Router::RouterSender broadcast();
    /**
        * @brief Reads packets from the ports and returns the next available packet.
        * @param localAddress The local address of the router.
        * @return An optional pair containing the port type and the communication packet, or std::nullopt if no packet is available.
        */

    [[nodiscard]] bool syncAllPorts() const;

    [[nodiscard]] std::optional<std::pair<IPort::PortType, acousea_CommunicationPacket*>> nextPacket(uint8_t localAddress) const;


    // ======================================================
    // Builder interno para API fluida
    // ======================================================
    class RouterSender
    {
    public:
        // friend class Router; // Permite a Router acceder a constructores privados
        friend Router::RouterSender Router::from(uint8_t) const;
        friend Router::RouterSender Router::broadcast();

    private:
        explicit RouterSender(const Router* router);

        explicit RouterSender(uint8_t sender, const Router* router);

    public:
        // Etapa 1: seleccionar puerto
        RouterSender& through(IPort::PortType type);

        // Etapa 2: enviar paquete
        // bool send(const acousea_CommunicationPacket* pkt) const;
        [[nodiscard]] bool send(acousea_CommunicationPacket& pkt) const;

    private:
        uint8_t senderAddress{broadcastAddress};
        IPort::PortType selectedPort{IPort::PortType::SerialPort};
        const Router* router;
    };

private:
#ifdef UNIT_TESTING
    friend class TestableRouter;
#endif

    std::vector<IPort*> ports_;

    [[nodiscard]] bool sendToPort(IPort::PortType port, const acousea_CommunicationPacket& packet) const;
};

#endif // COMMUNICATOR_RELAY_H
