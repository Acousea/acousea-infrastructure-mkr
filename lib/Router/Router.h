#ifndef COMMUNICATOR_RELAY_H
#define COMMUNICATOR_RELAY_H

#include <deque>
#include <map>
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

private:
    std::vector<IPort*> relayedPorts;

public:
    explicit Router(const std::vector<IPort*>& relayedPorts);

    void addRelayedPort(IPort* port);

    class RouterSender;

    [[nodiscard]] Router::RouterSender from(uint8_t sender) const;

    [[nodiscard]] Router::RouterSender broadcast();

    /**
        * @brief Reads packets from the ports and returns them grouped by port type.
        * @param localAddress The local address of the router.
        * @return A map where keys are port types and values are lists of packets received from those ports.
        */
    std::map<IPort::PortType, std::deque<acousea_CommunicationPacket>> readPorts(const uint8_t& localAddress);


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
        [[nodiscard]] bool send(const acousea_CommunicationPacket& pkt) const;

    private:
        uint8_t senderAddress{broadcastAddress};
        IPort::PortType selectedPort{IPort::PortType::SerialPort};
        const Router* router;
    };

private:
    // ---------------------- Packet encoding/decoding ----------------------
    static Result<acousea_CommunicationPacket> decodePacket(const std::vector<uint8_t>& raw);
    static Result<std::vector<uint8_t>> encodePacket(const acousea_CommunicationPacket& pkt);

    [[nodiscard]] bool sendToPort(IPort::PortType port, const acousea_CommunicationPacket& packet) const;
};

#endif // COMMUNICATOR_RELAY_H
