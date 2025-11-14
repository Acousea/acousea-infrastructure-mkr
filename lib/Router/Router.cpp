#include "Router.h"
#include <cinttypes>
#include <Logger/Logger.h>

#include "PacketQueue/PacketQueue.hpp"
#include "ProtoUtils/ProtoUtils.hpp"
#include "SharedMemory/SharedMemory.hpp"


namespace pb
{
    using ProtoUtils::CommunicationPacket::encodeInto;
    using ProtoUtils::CommunicationPacket::decodeInto;
}

Router::Router(const std::vector<IPort*>& ports,
               const std::vector<IPort::PortType>& relayedPortTypes,
               PacketQueue& packetQueue)
    : ports_(ports),
      relayedPortTypes_(relayedPortTypes),
      packetQueue_(packetQueue)
{
}

void Router::addPort(IPort* port)
{
    ports_.push_back(port);
}

void Router::addRelayedPortType(IPort::PortType portType)
{
    relayedPortTypes_.push_back(portType);
}

Router::RouterSender Router::from(const uint8_t sender) const
{
    return RouterSender(this, sender);
}


Router::RouterSender Router::broadcast()
{
    return RouterSender(this);
}

bool Router::syncAllPorts() const
{
    bool success = true;
    for (const auto& port : ports_)
    {
        if (!port->sync())
        {
            LOG_CLASS_ERROR("Router::syncAllPorts -> Failed to sync port of type %s",
                            IPort::portTypeToCString(port->getTypeEnum()));
            success = false;
        }
    }
    return success;
}


std::optional<std::pair<IPort::PortType, acousea_CommunicationPacket*>> Router::peekNextPacket(
    const uint8_t localAddress
) const
{
    // Recorre los puertos registrados
    for (const auto& port : ports_)
    {
        if (!port->available())
        {
            continue;
        }

        const auto readBuffer = reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer());
        constexpr auto readSize = SharedMemory::tmpBufferSize();


        const auto numReadBytes = packetQueue_.peekNext(
            port->getTypeU8(),
            readBuffer,
            readSize
        );

        if (numReadBytes == 0)
        {
            continue;
        }

        // Decodificar directamente en el buffer global
        const auto decodeResult = pb::decodeInto(
            readBuffer,
            numReadBytes,
            &SharedMemory::communicationPacketRef()
        );

        if (decodeResult.isError())
        {
            LOG_CLASS_ERROR("Router::nextPacket -> decode failed: %s", decodeResult.getError());
            // Discard the corrupt packet
            if (const auto discardOk = skipToNextPacket(port->getTypeEnum()); !discardOk)
            {
                LOG_CLASS_ERROR("Router::nextPacket -> discard packet failed: %s", decodeResult.getError());
            }
            continue;
        }

        acousea_CommunicationPacket& nextPacketRef = SharedMemory::communicationPacketRef();

        if (!nextPacketRef.has_routing)
        {
            LOG_CLASS_ERROR("Router::nextPacket -> packet has no routing info, Discarding...");
            // Discard the corrupt packet
            if (const auto discardOk = skipToNextPacket(port->getTypeEnum()); !discardOk)
            {
                LOG_CLASS_ERROR("Router::nextPacket -> discard packet failed: %s", decodeResult.getError());
            }
            continue;
        }

        const auto receiver = static_cast<uint8_t>(nextPacketRef.routing.receiver);
        if (receiver != localAddress && receiver != broadcastAddress)
        {
            LOG_CLASS_INFO(
                "Packet not for this node. Relaying through relayed ports and discarding (this=%d, receiver=%d)",
                localAddress, receiver);

            relayPacket(nextPacketRef);

            // Discard the packet
            if (const auto discardOk = skipToNextPacket(port->getTypeEnum()); !discardOk) // Discard the packet
            {
                LOG_CLASS_ERROR("Router::nextPacket -> discard packet failed: %s", decodeResult.getError());
            }
            continue;
        }

        // Paquete válido encontrado
        LOG_CLASS_INFO("Router::nextPacket -> OK (sender=%lu, receiver=%lu)", nextPacketRef.routing.sender,
                       nextPacketRef.routing.receiver);

        return std::make_pair(port->getTypeEnum(), &nextPacketRef);
    }

    // Ningún paquete válido encontrado
    return std::nullopt;
}

bool Router::skipToNextPacket(IPort::PortType portType) const
{
    const auto portU8 = static_cast<uint8_t>(portType);

    const bool skipOk = packetQueue_.skipToNextPacket(portU8);

    if (skipOk != 0)
    {
        LOG_CLASS_ERROR("::skipToNextPacket -> Failed to skip packet");
        return false;
    }

    return true;
}


bool Router::sendToPort(const IPort::PortType port, const acousea_CommunicationPacket& packet) const
{
    LOG_CLASS_FREE_MEMORY("::sendToPort() -> Encoding packet to send through port %s",
                          IPort::portTypeToCString(port)
    );
    const Result<size_t> resultBytesWritten = pb::encodeInto(
        packet,
        reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer()),
        SharedMemory::tmpBufferSize()
    );

    LOG_CLASS_FREE_MEMORY("::sendToPort() -> Packet encoded");

    if (resultBytesWritten.isError())
    {
        return false;
    }

    for (const auto& relayedPort : ports_)
    {
        if (relayedPort->getTypeEnum() == port)
        {
            return relayedPort->send(
                reinterpret_cast<const uint8_t*>(SharedMemory::tmpBuffer()),
                resultBytesWritten.getValueConst()
            );
        }
    }
    LOG_CLASS_FREE_MEMORY("::sendToPort() -> No matching port found, cannot send packet");

    LOG_CLASS_ERROR("Router::sendToPort() -> No relayed port found for type %s", IPort::portTypeToCString(port));

    return false;
}


void Router::relayPacket(const acousea_CommunicationPacket& inPacket) const
{
    for (const auto& portType : relayedPortTypes_)
    {
        if (const bool sendOk = sendToPort(portType, inPacket); !sendOk)
        {
            LOG_CLASS_ERROR("Router::relayPacket() -> Failed to relay packet through port %s",
                            IPort::portTypeToCString(portType));
        }
        else
        {
            LOG_CLASS_INFO("Router::relayPacket() -> Packet relayed successfully through port %s",
                           IPort::portTypeToCString(portType));
        }
    }
}

// --------------------------------------  Router::RouterSender --------------------------------------

Router::RouterSender::RouterSender(const Router* router) : router(router)
{
}

Router::RouterSender::RouterSender(const Router* router, const uint8_t sender_address) : router(router),
    senderAddress(sender_address)
{
}


// bool Router::RouterSender::send(const acousea_CommunicationPacket* pkt) const
// {
//     auto& packetRef = *pkt;
//     return router->sendToPort(selectedPort, packetRef);
// }


bool Router::RouterSender::send(acousea_CommunicationPacket& pkt) const
{
    // CHECK IF THE PACKET PREVIOUSLY HAD ROUTING INFO, TO SAVE THE DESTINATION
    const uint8_t destination = pkt.has_routing ? pkt.routing.sender : Router::originAddress;

    // SET ROUTING INFO
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = senderAddress;
    pkt.routing.receiver = destination;
    return router->sendToPort(selectedPort, pkt);
}

Router::RouterSender& Router::RouterSender::through(IPort::PortType type)
{
    selectedPort = type;
    return *this;
}

Router::RouterSender& Router::RouterSender::to(const uint8_t receiver)
{
    receiverAddress = receiver;
    return *this;
}
