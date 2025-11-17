#include "Logger/Logger.h"
#include "LogAndRelayErrorPacketRoutine.hpp"

LogAndRelayErrorPacketRoutine::LogAndRelayErrorPacketRoutine(Router& router)
    : IRoutine(getClassNameCString()), router(router)
{
}

Result<acousea_CommunicationPacket*> LogAndRelayErrorPacketRoutine::execute(
    acousea_CommunicationPacket* const optPacket)
{
    if (!optPacket) // Check for null pointer
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "No packet provided");
    }

    auto& inPacket = *optPacket;

    if (!inPacket.has_routing)
    {
        LOG_CLASS_WARNING("Packet has no routing info, skipping relay");
        return RESULT_SUCCESS(acousea_CommunicationPacket*, &inPacket);
    }
    if (const auto receiver = static_cast<uint8_t>(inPacket.routing.receiver); receiver != Router::broadcastAddress)
    {
        LOG_CLASS_WARNING("Packet not addressed to broadcast, skipping relay (receiver=%d)", receiver);
        return RESULT_SUCCESS(acousea_CommunicationPacket*, &inPacket);
    }
    LOG_CLASS_INFO("Relaying error packet through relayed ports...");
    router.relayPacket(inPacket);

    return RESULT_SUCCESS(acousea_CommunicationPacket*, &inPacket);
}

void LogAndRelayErrorPacketRoutine::reset()
{
    LOG_CLASS_INFO("::reset() -> Routine state reset.");
}
