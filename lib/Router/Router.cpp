#include "Router.h"

#include <Logger/Logger.h>
#include <pb_encode.h>
#include "pb_decode.h"


// Decodifica un buffer raw -> acousea_CommunicationPacket
Result<acousea_CommunicationPacket> Router::decodePacket(const std::vector<uint8_t>& raw)
{
    if (raw.empty())
    {
        return Result<acousea_CommunicationPacket>::failure("decodePacket: empty input buffer");
    }

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;

    pb_istream_t is = pb_istream_from_buffer(raw.data(), raw.size());
    if (!pb_decode(&is, acousea_CommunicationPacket_fields, &pkt))
    {
        return Result<acousea_CommunicationPacket>::failure(
            std::string("decodePacket: pb_decode failed: ") + PB_GET_ERROR(&is)
        );
    }

    return Result<acousea_CommunicationPacket>::success(pkt);
}

Result<std::vector<uint8_t>> Router::encodePacket(const acousea_CommunicationPacket& pkt)
{
    pb_ostream_t sizing = PB_OSTREAM_SIZING;
    if (!pb_encode(&sizing, acousea_CommunicationPacket_fields, &pkt))
    {
        Logger::logError(std::string("Router encodePacket(SIZE) -> ") + PB_GET_ERROR(&sizing));
        return Result<std::vector<uint8_t>>::failure(
            std::string("encodePacket (size): pb_encode failed: ") + PB_GET_ERROR(&sizing)
        );
    }

    std::vector<uint8_t> buf(sizing.bytes_written);
    pb_ostream_t os = pb_ostream_from_buffer(buf.data(), buf.size());
    if (!pb_encode(&os, acousea_CommunicationPacket_fields, &pkt))
    {
        Logger::logError(std::string("Router encodePacket(WRITE) -> ") + PB_GET_ERROR(&os));
        return Result<std::vector<uint8_t>>::failure(
            std::string("encodePacket (write): pb_encode failed: ") + PB_GET_ERROR(&os)
        );
    }
    return Result<std::vector<uint8_t>>::success(std::move(buf));
}


Router::Router(const std::vector<IPort*>& relayedPorts)
    : relayedPorts(relayedPorts)
{
}

void Router::addRelayedPort(IPort* port)
{
    relayedPorts.push_back(port);
}

Router::RouterSender Router::from(const uint8_t sender) const
{
    return RouterSender(sender, this);
}

Router::RouterSender Router::broadcast()
{
    return RouterSender(this);
}

std::map<IPort::PortType, std::deque<acousea_CommunicationPacket>> Router::readPorts(const uint8_t& localAddress)
{
    std::map<IPort::PortType, std::deque<acousea_CommunicationPacket>> receivedPackets = {};
    for (const auto& port : relayedPorts)
    {
        if (!port->available())
        {
            continue;
        }


        Logger::logInfo("Reading from port " + IPort::portTypeToString(port->getType()));
        std::vector<std::vector<uint8_t>> rawPacketBytes = port->read();


        for (const auto& rawData : rawPacketBytes)
        {
            Result<acousea_CommunicationPacket> decodedPacketResult = decodePacket(rawData);
            if (decodedPacketResult.isError())
            {
                Logger::logError("Router::readPorts() -> decode failed: " + decodedPacketResult.getError());
                continue;
            }

            const acousea_CommunicationPacket& packet = decodedPacketResult.getValue();

            // Si no hay routing, no podemos decidir destino
            if (!packet.has_routing)
            {
                Logger::logError("Router::readPorts() -> packet has no routing info. Ignoring packet...");
                continue;
            }


            const auto receiver = static_cast<uint8_t>(packet.routing.receiver);
            if (receiver != localAddress && receiver != broadcastAddress)
            {
                Logger::logInfo(
                    "Packet not for this node. This node: " + std::to_string(localAddress) +
                    " receiver: " + std::to_string(receiver)
                );
                continue;
            }

            // (Opcional) logging compacto del paquete
            Logger::logInfo("Router::readPorts() -> packet OK (sender=" +
                std::to_string(packet.routing.sender) + ", receiver=" +
                std::to_string(packet.routing.receiver) + ")");

            receivedPackets[port->getType()].push_back(packet);
        }
    }
    return receivedPackets;
}

bool Router::sendToPort(IPort::PortType port, const acousea_CommunicationPacket& packet) const
{
    const auto bytesPacketResult = encodePacket(packet);
    if (bytesPacketResult.isError()) return false;

    for (const auto& relayedPort : relayedPorts)
    {
        if (relayedPort->getType() == port)
        {
            return relayedPort->send(bytesPacketResult.getValueConst());
        }
    }
    Logger::logError("Router::sendToPort() -> No relayed port found for type " + IPort::portTypeToString(port));
    return false;
}


// --------------------------------------  Router::RouterSender --------------------------------------

Router::RouterSender::RouterSender(const Router* router) : router(router)
{
}

Router::RouterSender::RouterSender(const uint8_t sender, const Router* router) : senderAddress(sender), router(router)
{
}


bool Router::RouterSender::send(const acousea_CommunicationPacket& pkt) const
{
    acousea_CommunicationPacket modPkt = pkt;
    modPkt.has_routing = true;
    modPkt.routing = acousea_RoutingChunk_init_default;
    modPkt.routing.sender = senderAddress;
    return router->sendToPort(selectedPort, modPkt);
}

Router::RouterSender& Router::RouterSender::through(IPort::PortType type)
{
    selectedPort = type;
    return *this;
}
