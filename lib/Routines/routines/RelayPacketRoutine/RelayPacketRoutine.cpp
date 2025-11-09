#include "Logger/Logger.h"
#include "RelayPacketRoutine.hpp"

RelayPacketRoutine::RelayPacketRoutine(Router& router,
                                       const std::vector<IPort::PortType>& relayPorts)
    : IRoutine(getClassNameCString()),
      router(router),
      relayPorts(relayPorts)
{
}

Result<acousea_CommunicationPacket> RelayPacketRoutine::execute(
    const std::optional<acousea_CommunicationPacket>& optPacket)
{
    if (!optPacket.has_value())
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket,
                                     "RelayPacketRoutine: No packet provided");
    }

    const auto& pkt = optPacket.value();

    if (!pkt.has_routing)
    {
        LOG_CLASS_WARNING("RelayPacketRoutine: Packet has no routing info, skipping relay");
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket,
                                     "Packet has no routing info");
    }

    bool anySent = false;
    for (const auto& portType : relayPorts)
    {
        const bool sent = router
                          .from(static_cast<uint8_t>(pkt.routing.sender))
                          .through(portType)
                          .send(pkt);

        if (sent)
        {
            LOG_CLASS_INFO("RelayPacketRoutine: Relayed packet sender=%ld receiver=%ld through %s",
                           pkt.routing.sender,
                           pkt.routing.receiver,
                           IPort::portTypeToCString(portType));
            anySent = true;
        }
        else
        {
            LOG_CLASS_WARNING("RelayPacketRoutine: Failed to send packet through %s",
                              IPort::portTypeToCString(portType));
        }
    }

    if (!anySent)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket,
                                     "RelayPacketRoutine: Packet not relayed through any port");
    }

    return RESULT_SUCCESS(acousea_CommunicationPacket, pkt);
}
