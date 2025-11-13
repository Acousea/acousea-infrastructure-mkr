#include "Logger/Logger.h"
#include "RelayPacketRoutine.hpp"

RelayPacketRoutine::RelayPacketRoutine(Router& router,
                                       const std::vector<IPort::PortType>& relayPorts)
    : IRoutine(getClassNameCString()),
      router(router),
      relayPorts(relayPorts)
{
}

Result<acousea_CommunicationPacket*> RelayPacketRoutine::execute(acousea_CommunicationPacket* const optPacket)
{
    if (!optPacket) // Check for null pointer
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "RelayPacketRoutine: No packet provided");
    }

    auto& inPacket = *optPacket;

    if (!inPacket.has_routing)
    {
        LOG_CLASS_WARNING("RelayPacketRoutine: Packet has no routing info, skipping relay");
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "Packet has no routing info");
    }

    bool anySent = false;
    for (const auto& portType : relayPorts)
    {
        const bool sent = router
                          .from(static_cast<uint8_t>(inPacket.routing.sender))
                          .through(portType)
                          .send(inPacket);

        if (sent)
        {
            LOG_CLASS_INFO("RelayPacketRoutine: Relayed packet sender=%ld receiver=%ld through %s",
                           inPacket.routing.sender,
                           inPacket.routing.receiver,
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
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "RelayPacketRoutine: Packet not relayed through any port");
    }

    return RESULT_SUCCESS(acousea_CommunicationPacket*, &inPacket);
}
