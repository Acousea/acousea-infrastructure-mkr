#include "Router.h"
#include <cinttypes>
#include <Logger/Logger.h>
#include "ProtoUtils/ProtoUtils.hpp"
#include "SharedMemory/SharedMemory.hpp"


namespace pb
{
    using ProtoUtils::CommunicationPacket::encodeInto;
    using ProtoUtils::CommunicationPacket::decodeInto;
}

Router::Router(const std::vector<IPort*>& relayedPorts)
    : ports_(relayedPorts)
{
}

void Router::addRelayedPort(IPort* port)
{
    ports_.push_back(port);
}

Router::RouterSender Router::from(const uint8_t sender) const
{
    return RouterSender(sender, this);
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


std::optional<std::pair<IPort::PortType, acousea_CommunicationPacket*>> Router::nextPacket(
    const uint8_t localAddress) const
{
    // Recorre los puertos registrados
    for (const auto& port : ports_)
    {
        if (!port->available())
        {
            continue;
        }

        const auto readBuffer = reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer());
        const auto readSize = port->readInto(readBuffer, SharedMemory::tmpBufferSize());

        const auto numReadBytes = port->readInto(readBuffer, readSize);
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
            continue;
        }

        acousea_CommunicationPacket& nextPacketRef = SharedMemory::communicationPacketRef();

        if (!nextPacketRef.has_routing)
        {
            LOG_CLASS_ERROR("Router::nextPacket -> packet has no routing info, skipping.");
            continue;
        }

        const auto receiver = static_cast<uint8_t>(nextPacketRef.routing.receiver);
        if (receiver != localAddress && receiver != broadcastAddress)
        {
            LOG_CLASS_INFO("Packet not for this node. (this=%d, receiver=%d)", localAddress, receiver);
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


bool Router::sendToPort(const IPort::PortType port, const acousea_CommunicationPacket& packet) const
{
    const Result<size_t> resultBytesWritten = pb::encodeInto(
        packet,
        reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer()),
        SharedMemory::tmpBufferSize()
    );
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
    LOG_CLASS_ERROR("Router::sendToPort() -> No relayed port found for type %s", IPort::portTypeToCString(port));

    return false;
}


// --------------------------------------  Router::RouterSender --------------------------------------

Router::RouterSender::RouterSender(const Router* router) : router(router)
{
}

Router::RouterSender::RouterSender(const uint8_t sender, const Router* router) : senderAddress(sender), router(router)
{
}

// bool Router::RouterSender::send(const acousea_CommunicationPacket* pkt) const
// {
//     auto& packetRef = *pkt;
//     return router->sendToPort(selectedPort, packetRef);
// }


bool Router::RouterSender::send(acousea_CommunicationPacket& pkt) const
{
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = senderAddress;
    return router->sendToPort(selectedPort, pkt);
}

Router::RouterSender& Router::RouterSender::through(IPort::PortType type)
{
    selectedPort = type;
    return *this;
}
