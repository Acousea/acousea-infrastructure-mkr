#include "SetNodeConfigurationRoutine.h"

#include "Logger/Logger.h"

SetNodeConfigurationRoutine::SetNodeConfigurationRoutine(
    ModuleManager& moduleManager
)
    : IRoutine(getClassNameCString()),
      moduleManager(moduleManager)
{
}


Result<acousea_CommunicationPacket*> SetNodeConfigurationRoutine::execute(
    acousea_CommunicationPacket* const inPacketPtr) // Const pointer, not const data!
{
    if (!inPacketPtr) // Check for null pointer
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, ": No packet provided");
    }

    auto& inPacket = *inPacketPtr;

    if (inPacket.which_body != acousea_CommunicationPacket_command_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, " Packet is not of type command");
    }
    if (inPacket.body.command.which_command != acousea_CommandBody_setConfiguration_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     " Packet command is not of type setNodeConfiguration");
    }

    const auto& [modules_count, modules] = inPacket.body.command.command.setConfiguration;

    const Result<void> setResult = moduleManager.setModules(modules_count, modules);

    acousea_CommunicationPacket& outPacket = inPacket; // Reuse input packet for output (NOT COPY)
    outPacket.which_body = acousea_CommunicationPacket_response_tag;
    outPacket.body.response.which_response = acousea_ResponseBody_setConfiguration_tag;
    outPacket.body.response.response.setConfiguration = inPacket.body.command.command.setConfiguration;

    switch (setResult.getStatus())
    {
    case Result<void>::Type::Success:
        return RESULT_SUCCESS(acousea_CommunicationPacket*, &outPacket);
    case Result<void>::Type::Incomplete:
        return RESULT_CLASS_INCOMPLETEF(acousea_CommunicationPacket*, "%s", setResult.getError());
    case Result<void>::Type::Failure:
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "%s", setResult.getError());
    }
}
